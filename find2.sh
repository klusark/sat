
c=0

while true; do
    ./random 4 > out/puzzle$c
    /usr/bin/time -f %e ./direct < out/puzzle$c 2> out/time$c
    if [ $? -ne 0 ] ; then
        rm out/time$c
        rm out/puzzle$c
    else
        val=$(echo `cat out/time$c`'<0.10' | bc)
        if [ $val -ne 0 ] ; then
            rm out/time$c
            rm out/puzzle$c
        fi
    fi
    c=$[$c+1]
done
