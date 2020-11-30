#ifndef _RMEM_H_
#define _RMEM_H_

// remote_mem api

#define RMEM_ACCESS_READ  (1)
#define RMEM_ACCESS_WRITE (2)

struct remote_mem {
    void * buf;
    void * meta;
    int access;
};

// remote APIs
struct remote_mem * rinit(int access, size_t size, void *args);
int rclose(struct remote_mem * rmem);

void * rcreatebuf (struct remote_mem * rmem, size_t size);
int rread (struct remote_mem * rmem, void *buf, uint64_t addr, size_t size);
int rwrite (struct remote_mem * rmem, void *buf, uint64_t addr, size_t size);
int ralloc (struct remote_mem * rmem, void *buf, uint64_t addr, size_t size);

#endif
