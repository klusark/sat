for i in $(seq 1 5 256)
do
    for j in {1..10}
    do
        ./run 4 200 $i
    done
done
