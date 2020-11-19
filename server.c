#include <infiniband/verbs.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "test.h"

int server() {

    struct rdma_conn conn;
    memset(&conn, 0, sizeof(struct rdma_conn));
    conn.port = config.server.port;

    // TODO: goto common
    create_context(&config.server, &conn);

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

    // create MRs
    for (int i = 0; i < config.server_num_mr; i++)
        create_mr(&conn, config.server_mr_size,
          IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE | IBV_ACCESS_REMOTE_READ);

    // Exchange with server
    server_exchange_info(&conn);

    // Enable QP, server only need to get to RTR
    qp_stm_reset_to_init(&conn);
    qp_stm_init_to_rtr(&conn);

    // clean up
    for (int i = 0; i < conn.num_mr; i++) {
        ibv_dereg_mr(conn.mr + i);
    }
    ibv_close_device(conn.context);

    return 0;
}

int main(int argc, char *argv[]) {
    parse_config(argc, argv);
    return server();
}
