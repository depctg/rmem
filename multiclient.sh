mkdir -p $1
for i in $(seq 1 $2)
do
    ./arrayclient 2> /dev/null > $1/$i.result&
    # set core for pid
    taskset -cp $i $!
done
