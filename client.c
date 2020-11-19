#include <infiniband/verbs.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "test.h"

static int client_simple_write(struct rdma_conn *conn);

int client() {

    struct rdma_conn conn;
    memset(&conn, 0, sizeof(struct rdma_conn));
    conn.port = config.client.port;

    // TODO: goto common
    create_context(&config.client, &conn);

    // Create PD
    conn.pd = ibv_alloc_pd(conn.context);
    if (conn.pd == NULL) {
        printf("iDIE");
        return -1;
    }

    // Create CQ
    conn.cq = ibv_create_cq(conn.context, config.cq_size, NULL, NULL, 0);

    // Create QP
    create_qp(&conn);

    // Exchange with server
    client_exchange_info(&conn);

    // Enable QP
    qp_stm_reset_to_init(&conn);
    qp_stm_init_to_rtr(&conn);
    qp_stm_rtr_to_rts(&conn);

    // We need a buffer at client side
    create_mr(&conn, config.client_mr_size, IBV_ACCESS_LOCAL_WRITE);

    // client write..
    client_simple_write(&conn);

    // clean up
    for (int i = 0; i < conn.num_mr; i++) {
        ibv_dereg_mr(conn.mr + i);
    }
    ibv_close_device(conn.context);

    return 0;
}

static int client_simple_write(struct rdma_conn *conn){
    int ret;

    // SGE for request, we use only 1
    struct ibv_sge* sge = (struct ibv_sge *)calloc(1, sizeof(struct ibv_sge));
    sge->addr = (uint64_t)conn->mr[0].addr;
    sge->length = config.request_size;
    sge->lkey = conn->mr[0].lkey;

    struct ibv_send_wr wr;
    memset(&wr, 0, sizeof(wr));
    // user tag
    wr.wr_id = 233;
    wr.sg_list = sge;
    wr.num_sge = 1;

    wr.wr.rdma.remote_addr = (uint64_t)conn->peerinfo->mr[0].addr;
    wr.wr.rdma.rkey = conn->peerinfo->mr[0].rkey;
    wr.opcode = IBV_WR_RDMA_WRITE;

    wr.send_flags = IBV_SEND_SIGNALED;
    wr.next = NULL;

    // post send
    // TODO: add time here
    ret = ibv_post_send(conn->qp, &wr, NULL);

    // PULL for result...
    struct ibv_wc wc;
    int bytes;

    while ((bytes = ibv_poll_cq(conn->cq, 1, &wc)) == 0) ;
    // TODO: add time here

    if (bytes > 0 && wc.status == IBV_WC_SUCCESS) {
        printf("GET REPLY!\n");
    }
    
    free(sge);
}

int main(int argc, char *argv[]) {
    parse_config(argc, argv);
    return client();
}
