#/usr/bin/env bash
#$ -S /bin/bash
# ^ Second line is for use in rocks-based clusters


#
#  Generates a *FILLED* sudoku
#  

INSTANCESDIR=""
BINPATH=.
PREFIX="sudoku-"

if [ -n "$INSTANCESDIR" ]
	then	
		cd $INSTANCESDIR
	fi

ROW=$1
COLUMN=$2
SHUFFLES=$3
SEED=$4

FILE=$PREFIX$ROW"x"$COLUMN""
prod=`bc -l << EOF
scale=0
$ROW*$COLUMN
EOF
`
$BINPATH/sudokugenerator  newsudoku $FILE $prod $ROW
$BINPATH/sudokugenerator  shuffle   $FILE 1 $FILE $SHUFFLES $SEED


