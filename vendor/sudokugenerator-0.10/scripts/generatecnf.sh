#/usr/bin/env bash
#$ -S /bin/bash
# ^ Second line is for use in rocks-based clusters



#
# Converts a .pls file to a .cnf file
#
# 


INSTANCESDIR=""
BINPATH=.

if [ -n "$INSTANCESDIR" ]
	then	
		cd $INSTANCESDIR
	fi

FILE=$1

$BINPATH/sign_encode sudokupls2sat  $FILE


