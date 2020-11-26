#!/bin/bash

rsync -r ~/rdma-test node1:rdma-test
# rsync -r ~/rdma-test node2:rdma-test
# rsync -r ~/rdma-test node3:rdma-test
rsync -r ~/rdma-test node4:rdma-test
rsync -r ~/rdma-test node5:rdma-test

