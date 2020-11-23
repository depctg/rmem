#include "test.h"
#include <string.h>

struct config config;

int parse_config(int argc, char *argv[]) {
    // fill in config here
    memset(&config, 0, sizeof(struct config));

    config.cq_size = 16;

    config.client_mr_size = 4096;
    config.server_num_mr = 1024 * 8;
    config.server_mr_size = 8192;

    // set ib info
    config.server.num_devices = 2;
    config.server.port = 1;
    config.server.device_name = "mlx5_1";

    config.client.num_devices = 2;
    config.client.port = 1;
    config.client.device_name = "mlx5_1";

    // test parameters
    config.request_size = 64;
    config.server_url = "tcp://wuklab-01.ucsd.edu:2345";
    config.server_listen_url = "tcp://*:2345";

    config.server_enable_odp = 0;
    config.server_multi_conn = 1;

    return 0;
}
