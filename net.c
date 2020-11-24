#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <infiniband/verbs.h>
#include <nanomsg/nn.h>
#include <nanomsg/reqrep.h>

#include "test.h"

static void fatal(const char *func)
{
        fprintf(stderr, "%s: %s\n", func, nn_strerror(nn_errno()));
        exit(1);
}

static int extract_info(struct rdma_conn *conn, void **buf) {

    int info_size = sizeof(struct conn_info) + conn->num_mr * sizeof(struct ibv_mr);
    *buf = calloc(1, info_size);
    struct conn_info *info = (struct conn_info*) *buf;

    if (config.use_roce) {
        union ibv_gid gid;
        ibv_query_gid(conn->context, conn->port, config.gid_idx, &gid);
        info->gid = gid;
    } else {
        struct ibv_port_attr port_attr;
        ibv_query_port(conn->context, conn->port, &port_attr);
        info->local_id = port_attr.lid;
    }

    info->port = conn->port;
    info->qp_number = conn->qp->qp_num;
    info->num_mr = conn->num_mr;
    if (conn->num_mr != 0) 
        memcpy(&info->mr, conn->mr, conn->num_mr * sizeof(struct ibv_mr));

    return info_size;
}

int client_exchange_info(struct rdma_conn *conn) {
    int sock, rv;
    int send_size, bytes;
    void *info, *peerinfo = NULL;
    int size = 1024*1024*64;


    printf("connecting to server %s...\n", config.server_url);
    if ((sock = nn_socket(AF_SP, NN_REQ)) < 0) {
        fatal("nn_socket");
    }

    if (nn_setsockopt(sock, NN_SOL_SOCKET, NN_RCVBUF, &size, sizeof(size)) != 0) {
        fatal("nng_setopt_size");
    }
    if (nn_setsockopt(sock, NN_SOL_SOCKET, NN_SNDBUF, &size, sizeof(size)) != 0) {
        fatal("nng_setopt_size");
    }
    size = -1;
    if (nn_setsockopt(sock, NN_SOL_SOCKET, NN_RCVMAXSIZE, &size, sizeof(size)) != 0) {
        fatal("nng_setopt_size");
    }

    if ((rv = nn_connect(sock, config.server_url)) < 0) {
        fatal("nn_connect");
    }

    send_size = extract_info(conn, &info);
    printf("try to send size %d\n", send_size);
    if ((bytes = nn_send(sock, info, send_size, 0)) < 0) {
        fatal("nn_send");
    }
    if ((bytes = nn_recv(sock, &peerinfo, NN_MSG, 0)) < 0) {
        fatal("nn_recv");
    }

    conn->peerinfo = peerinfo;
    printf("received mr, with bytes %d, num %d\n", bytes, conn->peerinfo->num_mr);

    // TODO: check this
    // nn_freemsg(buf);

    free(info);

    return 0;
}

int server_exchange_info(struct rdma_conn *conn) {
    int sock, rv;
    int send_size, bytes;
    void *info, *peerinfo = NULL;
    int size = 1024*1024*64;

    // create server
    if ((sock = nn_socket(AF_SP, NN_REP)) < 0) {
        fatal("nn_socket");
    }

    if (nn_setsockopt(sock, NN_SOL_SOCKET, NN_RCVBUF, &size, sizeof(size)) != 0) {
        fatal("nng_setopt_size");
    }
    if (nn_setsockopt(sock, NN_SOL_SOCKET, NN_SNDBUF, &size, sizeof(size)) != 0) {
        fatal("nng_setopt_size");
    }
    size = -1;
    if (nn_setsockopt(sock, NN_SOL_SOCKET, NN_RCVMAXSIZE, &size, sizeof(size)) != 0) {
        fatal("nng_setopt_size");
    }

    if ((rv = nn_bind(sock, config.server_listen_url)) < 0) {
        fatal("nn_bind");
    }

    if ((bytes = nn_recv(sock, &peerinfo, NN_MSG, 0)) < 0) {
        fatal("nn_recv");
    }

    // TODO: fix this, use memcpy
    conn->peerinfo = peerinfo;
    send_size = extract_info(conn, &info);
    if ((bytes = nn_send(sock, info, send_size, 0)) < 0) {
        fatal("nn_send");
    }
    printf("send %d bytes, expect to send %d bytes \n", send_size, bytes);

    // TODO: check this
    // nn_freemsg(buf);

    return nn_shutdown(sock, rv);
}

