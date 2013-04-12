./scripts/generatesudoku.sh $1 $1 $2 singly $3 400 > /dev/null
../../fixformat < sudoku-${1}x${1}.pls > puzzle
../../sudoku2sat < puzzle > cnf1
../../sudoku2sat2 < puzzle > cnf2
time="/usr/bin/time -f %e timeout 15"
#echo "cnf1 + minisat"
$time ../../vendor/minisat/core/minisat < cnf1 > /dev/null 2>> data.txt
#echo "cnf2 + minisat"
$time ../../vendor/minisat/core/minisat < cnf2 > /dev/null 2>> data.txt
#echo "cnf1 + glucose"
$time ../../vendor/glucose2.2/core/glucose < cnf1 > /dev/null 2>> data.txt
#echo "cnf2 + glucose"
$time ../../vendor/glucose2.2/core/glucose < cnf2 > /dev/null 2>> data.txt
#echo "cnf1 + lingeling"
$time ../../vendor/lingeling-ala-b02aa1a-121013/lingeling < cnf1 > /dev/null 2>> data.txt
#echo "cnf2 + lingeling"
$time ../../vendor/lingeling-ala-b02aa1a-121013/lingeling < cnf2 > /dev/null 2>> data.txt
#echo "direct"
$time ../../direct < puzzle > /dev/null 2>> data.txt


