#include "test.h"
#include <string.h>

struct config config;

int parse_config(int argc, char *argv[]) {
    // fill in config here
    memset(&config, 0, sizeof(struct config));

    config.cq_size = 16;

    config.client_mr_size = 4096;
    config.server_num_mr = 1;
    config.server_mr_size = 1024 * 1024 * 4;

    // set ib info
    config.server.num_devices = 2;
    config.server.port = 0;
    config.server.device_name = "mlx5_1";

    config.client.num_devices = 2;
    config.client.port = 0;
    config.client.device_name = "mlx5_1";

    config.request_size = 64;
    config.server_url = "tcp://wuklab-01.ucsd.edu:2345";

    return 0;
}
