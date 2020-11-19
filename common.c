#include <string.h>
#include <infiniband/verbs.h>
#include <stdio.h>
#include <stdlib.h>
#include "test.h"


int create_context(struct epinfo *ep, struct rdma_conn *conn) {
    int num_devices = ep->num_devices;
    char * device_name = ep->device_name;
    struct ibv_device** device_list = ibv_get_device_list(&num_devices);
    for (int i = 0; i < num_devices; i++){
        if (strcmp(device_name, ibv_get_device_name(device_list[i])) == 0) {
            conn->context = ibv_open_device(device_list[i]);
            break;
        }
    }

    ibv_free_device_list(device_list);
    if (conn->context == NULL) {
        printf("iDIE");
        return -1;
    }
}

int create_qp(struct rdma_conn *conn) {
    struct ibv_qp_init_attr qp_init;
    memset(&qp_init, 0, sizeof(qp_init));
    qp_init.qp_type = IBV_QPT_RC;
    qp_init.sq_sig_all = 1;       // always work completion
    qp_init.send_cq = conn->cq;         // completion queue for cq
    qp_init.recv_cq = conn->cq;         // completion queue for sq
    qp_init.cap.max_send_wr = 1; 
    qp_init.cap.max_recv_wr = 1;
    // TODO: set other ATTRs

    conn->qp = ibv_create_qp(conn->pd, &qp_init);
    if (conn->qp == NULL) {
        printf("iDIE");
        return -1;
    }
}

int create_mr(struct rdma_conn *conn, int size, int access) {

    int ret;
    void * buffer = malloc(size);
    struct ibv_mr *mr;

    if (buffer == NULL) {
        printf("malloc fail");
        return -1;
    }

    mr = ibv_reg_mr(conn->pd, buffer, size, access);
    if (mr == NULL) {
        printf("register MR fail");
        return -1;
    }

    conn->num_mr += 1;
    conn->mr = reallocarray(conn->mr, conn->num_mr, sizeof(struct ibv_mr));
    if (conn->mr == NULL) {
        printf("realloc fail");
        return -1;
    }

    return 0;
}

int qp_stm_reset_to_init(struct rdma_conn *conn) {
    int ret;
    struct ibv_qp_attr qp_attr;

    memset(&qp_attr, 0, sizeof(qp_attr));
    qp_attr.qp_state = IBV_QPS_INIT;
    qp_attr.port_num = conn->port;
    qp_attr.pkey_index = 0;
    qp_attr.qp_access_flags = IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_READ | IBV_ACCESS_REMOTE_WRITE;

    // TODO: match here with attrs
    ret = ibv_modify_qp(conn->qp, &qp_attr, IBV_QP_STATE | IBV_QP_PKEY_INDEX | IBV_QP_PORT | IBV_QP_ACCESS_FLAGS);
    if (ret != 0) {
        printf("init fail");
        return -1;
    }
    return 0;
}

int qp_stm_init_to_rtr(struct rdma_conn *conn) {

    int ret;
    struct ibv_qp_attr qp_attr;
    memset(&qp_attr, 0, sizeof(qp_attr));

    qp_attr.qp_state = IBV_QPS_RTR;
    qp_attr.path_mtu = IBV_MTU_1024;
    qp_attr.rq_psn = 0;
    qp_attr.max_dest_rd_atomic = 1;
    qp_attr.min_rnr_timer = 0x12;

    qp_attr.ah_attr.is_global = 0;
    qp_attr.ah_attr.sl = 0;
    qp_attr.ah_attr.src_path_bits = 0;
    qp_attr.ah_attr.port_num = conn->peerinfo->port;

    qp_attr.dest_qp_num = conn->peerinfo->qp_number;
    qp_attr.ah_attr.dlid = conn->peerinfo->local_id;

    ret = ibv_modify_qp(conn->qp, &qp_attr, IBV_QP_STATE | IBV_QP_AV | IBV_QP_PATH_MTU | IBV_QP_DEST_QPN | IBV_QP_RQ_PSN | IBV_QP_MAX_DEST_RD_ATOMIC | IBV_QP_MIN_RNR_TIMER);

    if (ret != 0) {
        printf("rtr fail");
        return -1;
    }

    return 0;
}

int qp_stm_rtr_to_rts(struct rdma_conn *conn) {

    int ret;
    struct ibv_qp_attr qp_attr;
    memset(&qp_attr, 0, sizeof(qp_attr));

    qp_attr.qp_state = IBV_QPS_RTS;
    qp_attr.sq_psn = 0;
    ret = ibv_modify_qp(conn->qp, &qp_attr, IBV_QP_STATE | IBV_QP_SQ_PSN);
    if (ret != 0) {
        printf("rts fail");
        return -1;
    }

    return 0;
}
