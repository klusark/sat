#/usr/bin/env bash
#$ -S /bin/bash
# ^ Second line is for use in rocks-based clusters


#
#  Generates a sudoku 
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
METHOD=$4
HOLES=$5
SHUFFLES2=$6
SEED=$7

FILE=$PREFIX$ROW"x"$COLUMN""
prod=`bc -l << EOF
scale=0
$ROW*$COLUMN
EOF
`
$BINPATH/sudokugenerator  newsudoku $FILE $prod $ROW
$BINPATH/sudokugenerator  shuffle   $FILE 1 $FILE $SHUFFLES $SEED

if [ -z "$METHOD" ]
	then 
		echo "Bad method"
		exit -1
	fi

case "$METHOD" in 
	"singly")
		$BINPATH/sudokugenerator poke singlybalanced $FILE $HOLES $FILE $SEED
		;;

	"doubly")
		$BINPATH/sudokugenerator poke doublybalanced $FILE $HOLES $SHUFFLES2 $FILE $SEED
		;;

	"fully")
		$BINPATH/sudokugenerator poke fullybalanced $FILE $HOLES $SHUFFLES2 $FILE $SEED
		;;
	"*")
		echo "Bad method"
		exit -1
		;;
esac 

