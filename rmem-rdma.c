#include "rmem.h"
#include "test.h"

#define RMEM_RDMA_WRID_READ  101
#define RMEM_RDMA_WRID_WRITE 102

// One RMEM maps to a single MR, fornow
struct remote_mem_rdma {
    struct remote_mem rmem;
    struct rdma_conn conn;
}

static inline struct ibv_mr * get_remote_mr_by_addr(struct remote_mem_rmda * rmem) {
    // TODO: for now, we always return mr 0;
    if (rmem->conn->peerinfo && rmem->conn->peerinfo->num_mr != 0)
        return rmem->conn->peerinfo->mr
    else
        return NULL
}

// Different kind of remote buffers will handle different MRs
// parameter: target memory
struct remote_mem * rinit(int access, void *args) {
    struct remote_mem_rdma * rmem;

    // the target 
    char * url = (char *) args;

    // TODO: check ROCE here?
    rmem = (struct remote_mem_rdma *)calloc(1, sizeof(struct remote_mem_rdma));
    rmem->access = access;

    if (config.use_roce)
        rmem->conn.gid = config.gid;

    // This port is rnic port
    rmem->conn.port = config.client.port;

    // TODO: goto common
    create_context(&config.client, &conn);

    // Create PD
    conn.pd = ibv_alloc_pd(conn.context);
    if (conn.pd == NULL) {
        printf("create pd fail");
        return -1;
    }

    // Create CQ
    conn.cq = ibv_create_cq(conn.context, config.cq_size, NULL, NULL, 0);

    // Create QP
    create_qp(&conn);

    // Exchange with server
    client_exchange_info(&conn, url);

    // Enable QP
    qp_stm_reset_to_init(&conn);
    qp_stm_init_to_rtr(&conn);
    qp_stm_rtr_to_rts(&conn);

    return (struct remote_mem *)rmem;
}

void * rcreatebuf (struct remote_mem * rmem, void *buf, size_t size) {
    int ret;

    // We need a buffer at client side
    ret = create_mr(&rmem->conn, size, IBV_ACCESS_LOCAL_WRITE);
    if (ret != 0) {
        return -1;
    }

    return rmem->conn.mr[rmem->conn.num_mr - 1];
}

int rread (struct remote_mem * _rmem, void *buf, uint64_t addr, size_t size) {
    int ret;
    struct ibv_wc wc;
    struct ibv_sge * sge;
    struct ibv_mr * mr;
    struct ibv_send_wr wr, *badwr = NULL;

    struct rdma_conn *conn = &((struct remote_mem_rdma *)rmem)->conn;

    sge = (struct ibv_sge *)calloc(1, sizeof(struct ibv_sge));
    mr = get_mr_by_addr(rmem, addr);

    sge->addr = (uint64_t)conn->mr[0].addr + addr;
    sge->length = size;
    sge->lkey = conn->mr[0].lkey;

    memset(&wr, 0, sizeof(wr));
    // user tag
    wr.wr_id = RMEM_RDMA_WRID_READ;
    wr.sg_list = sge;
    wr.num_sge = 1;

    wr.wr.rdma.remote_addr = (uint64_t)addr;
    wr.wr.rdma.rkey = conn->peerinfo->mr[0].rkey;
    wr.opcode = IBV_WR_RDMA_READ;

    wr.send_flags = IBV_SEND_SIGNALED;
    wr.next = NULL;

    ibv_post_send(conn->qp, &wr, &badwr);

    // TODO: WHEN DO WE POLL?
    int need_poll = 1;
    while (need_poll) {
        while ((ret = ibv_poll_cq(conn->cq, 1, &wc)) == 0) {
            break;
        }
    }

    // TODO: free SGE?
    return 0;
}

int rwrite (struct remote_mem * rmem, void *buf, uint64_t addr, size_t size) {
    int ret;
    struct ibv_wc wc;
    struct ibv_sge* sge;
    struct ibv_send_wr wr, *badwr = NULL;

    struct rdma_conn *conn = &((struct remote_mem_rdma *)rmem)->conn;

    sge = (struct ibv_sge *)calloc(1, sizeof(struct ibv_sge));


    sge->addr = (uint64_t)conn->mr[0].addr + addr;
    sge->length = size;
    sge->lkey = conn->mr[0].lkey;

    memset(&wr, 0, sizeof(wr));
    // user tag
    wr.wr_id = RMEM_RDMA_WRID_READ;
    wr.sg_list = sge;
    wr.num_sge = 1;

    wr.wr.rdma.remote_addr = (uint64_t)addr;
    wr.wr.rdma.rkey = conn->peerinfo->mr[0].rkey;
    wr.opcode = IBV_WR_RDMA_READ;

    wr.send_flags = IBV_SEND_SIGNALED;
    wr.next = NULL;

    ibv_post_send(conn->qp, &wr, &badwr);

    // TODO: WHEN DO WE POLL?
    int need_poll = 1;
    while (need_poll) {
        while ((ret = ibv_poll_cq(conn->cq, 1, &wc)) == 0) {
            break;
        }
    }

    // TODO: free SGE?
    return 0;
}

// currently, we do not support ralloc
int ralloc (struct remote_mem * rmem, void *buf, uint64_t addr, size_t size) {
    fprintf(stderr, "ralloc is not supportted on RMEM-RDMA\n");
    return -1;
}
