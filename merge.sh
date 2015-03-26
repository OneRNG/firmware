#!/bin/sh
# $1 incoming .ihx file
# $2 random_data
# $3 size
# $4 output file
#
#	makebin comes from the sdcc build system
#
	makebin -p $1 $1.tmp
	cat $1.tmp $2 >$4
	n=$(($3-600-80))
	truncate -s $n $4
	rm $1.tmp
	exit 0
