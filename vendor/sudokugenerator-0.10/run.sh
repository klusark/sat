./scripts/generatesudoku.sh $1 $1 $2 singly $3 400 > /dev/null
../../fixformat < sudoku-${1}x${1}.pls > puzzle
../../sudoku2sat < puzzle > cnf1
../../sudoku2sat2 < puzzle > cnf2
echo "cnf1 + minisat"
time ../../vendor/minisat/core/minisat < cnf1 > /dev/null
echo "cnf2 + minisat"
time ../../vendor/minisat/core/minisat < cnf2 > /dev/null
echo "cnf1 + glucose"
time ../../vendor/glucose2.2/core/glucose < cnf1 > /dev/null
echo "cnf2 + glucose"
time ../../vendor/glucose2.2/core/glucose < cnf2 > /dev/null
echo "direct"
time ../../direct < puzzle > /dev/null
