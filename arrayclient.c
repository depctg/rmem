#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>

#include "jpeglib.h"

#include "test.h"
#include "rmem.h"
#include "rarray.h"

const int buffer_size = 1024*1024*4;
const int num_iter = 1024 * 16;

static int job(void * sbuf, void *tbuf);
void * threadtask(void * args);

pthread_barrier_t barr;

// program
int main(int argc, char *argv[]) {

    parse_config(argc, argv);

    pthread_t threads[1024];
    int num_threads = config.threads;
    cpu_set_t cpus;

    pthread_barrier_init(&barr, NULL, num_threads);
    for (int i = 0; i < num_threads; i++) {
            CPU_ZERO(&cpus);
	    CPU_SET(i, &cpus);

	    pthread_create(&threads[i], NULL, threadtask, NULL);
	    pthread_setaffinity_np(threads[i], sizeof(cpu_set_t), &cpus);
    }

    for (int i = 0; i < num_threads; i++)
        pthread_join(threads[i], NULL);
    return 0;

} 

void * threadtask(void * args) {
    void *rbuf, *wbuf;
    struct remote_mem *rarray, *warray;
    struct timespec tstart={0,0}, tend={0,0};

    // create array
    rarray = rinit(RMEM_ACCESS_READ,  (size_t)1024*1024, config.server_rdma_read_url);
    rbuf = rcreatebuf(rarray, buffer_size);

    warray = rinit(RMEM_ACCESS_WRITE, (size_t)1024*1024, config.server_rdma_write_url);
    wbuf = rcreatebuf(warray, buffer_size);

    pthread_barrier_wait(&barr);

    clock_gettime(CLOCK_MONOTONIC, &tstart);
    for (size_t i = 0; i < num_iter; i++) {
        rarray_read(rarray, rbuf, 0, config.jpg_size);

        // printf("get cat at %p, %02x %02x\n", rbuf, ((char*)rbuf)[0], ((char*)rbuf)[1]);

	// for (int i= 0; i < 15; i++) job(rbuf,wbuf);
        int output_size = job(rbuf, wbuf);

        // printf("decode size %d\n", output_size);

        rarray_write(warray, wbuf, 0, output_size);
    }
    clock_gettime(CLOCK_MONOTONIC, &tend);

    uint64_t ns = (uint64_t) tend.tv_sec * 1000000000ULL -
                  (uint64_t) tstart.tv_sec * 1000000000ULL +
                  tend.tv_nsec - tstart.tv_nsec;

    printf("%lu\n", ns);

}

// simple copy..
#if 0
static void job(void * sbuf, void *tbuf) {
	const char * message = "This is a message...";
    memcpy(sbuf, message, strlen(message) + 1);
    memcpy(tbuf, sbuf, buffer_size);
}
#endif

static int job(void * sbuf, void *tbuf) {
    struct jpeg_error_mgr jerr;
    struct jpeg_decompress_struct cinfo;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    jpeg_mem_src(&cinfo, sbuf, config.jpg_size);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    int width = cinfo.output_width;
    int height = cinfo.output_height;
    int pixel_size = cinfo.output_components;

    int bmp_size = width * height * pixel_size;
    int row_stride = width * pixel_size;

    while (cinfo.output_scanline < cinfo.output_height) {
        unsigned char *buffer_array[1];
        buffer_array[0] = tbuf + (cinfo.output_scanline) * row_stride;
        jpeg_read_scanlines(&cinfo, buffer_array, 1);
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    return bmp_size;
}


