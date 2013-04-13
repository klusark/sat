../sudoku2sat < $1 > cnf1
../sudoku2sat2 < $1 > cnf2
../sgen < $1 > out.S
time="/usr/bin/time -f %e timeout 10"
#echo "cnf1 + minisat"
$time ../vendor/minisat/core/minisat < cnf1 > /dev/null 2>> data.txt
#echo "cnf2 + minisat"
$time ../vendor/minisat/core/minisat < cnf2 > /dev/null 2>> data.txt
#echo "cnf1 + glucose"
$time ../vendor/glucose2.2/core/glucose < cnf1 > /dev/null 2>> data.txt
#echo "cnf2 + glucose"
$time ../vendor/glucose2.2/core/glucose < cnf2 > /dev/null 2>> data.txt
#echo "cnf1 + lingeling"
$time ../vendor/lingeling-ala-b02aa1a-121013/lingeling < cnf1 > /dev/null 2>> data.txt
#echo "cnf2 + lingeling"
$time ../vendor/lingeling-ala-b02aa1a-121013/lingeling < cnf2 > /dev/null 2>> data.txt
#echo "direct"
$time ../vendor/clasp-2.1.3/build/release/bin/clasp < cnf1 > /dev/null 2>> data.txt
$time ../vendor/clasp-2.1.3/build/release/bin/clasp < cnf2 > /dev/null 2>> data.txt
$time ../direct < $1 > /dev/null 2>> data.txt
$time ~/Enfragmo ../S.T out.S > /dev/null 2>> data.txt
../tocsv 10 < data.txt
rm data.txt
