
./a.out $2 $3 > board${1}.txt
./sudoku2sat < board${1}.txt > out${1}.cnf
./SatELite_release < out${1}.cnf > out${1}.results
grep UNSATISFIABLE out${1}.results
if [ $? -eq 0 ]; then
    ./find.sh $1 $2 $3
fi
