mkdir -p $1
for i in $(seq 1 $2)  
do   
    ~/rdma-test/rdma-test/client > $1/$i.result&
done
