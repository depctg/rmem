#include <string.h>

#include "test.h"
#include "rmem.h"
#include "rarray.h"

const int buffer_size = 1024*1024*4;

static void job(void * sbuf, void *tbuf);

// program
int main() {
    void *rbuf, *wbuf;
    int num_iter = 100;
    struct remote_mem *rarray, *warray;

    // create array
    rarray = rinit(RMEM_ACCESS_READ,  (size_t)1024*1024*1024, config.server_rdma_read.url);
    rbuf = rcreatebuf(rmem, buffer_size);

    warray = rinit(RMEM_ACCESS_WRITE, (size_t)1024*1024*1024, config.server_rdma_write.url);
    wbuf = rcreatebuf(rmem, buffer_size);

    for (size_t i = 0; i < num_iter; i++) {
        rarray_read(rarray, rbuf, i, RARRAY_SZ_CELL);
        job(rbuf, wbuf);
        rarray_write(warrray, wbuf, i, RARRAY_SZ_CELL);
    }

}

static void job(void * sbuf, void *tbuf) {
    memcpy(tbuf, sbuf, buffer_size);
}
