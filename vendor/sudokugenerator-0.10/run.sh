./scripts/generatesudoku.sh $1 $1 $2 singly $3 > /dev/null
../../fixformat < sudoku-${1}x${1}.pls > puzzle
../../sudoku2sat < puzzle > cnf
time ../../vendor/minisat/core/minisat < cnf
