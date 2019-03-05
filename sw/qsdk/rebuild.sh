#!/usr/bin/env bash

NOW=$(date +%y%m%d-%H%M%S)
START=$(date +%y-%m/%d-%H/%M/%S)
make V=s 2>&1 > build_log_$NOW.txt


if [ "$?" = "0" ] ; then
	echo "========================"
	echo "BUILD SUCCESS!!!"
	echo "========================"
else
	echo "========================"
	echo "BUILD FAILED!!!"
	echo "========================"
	exit 1
fi

echo "========================"
echo "Makeing Single Image...."
cd ../../src
./single_image.sh $1
echo "Makeing Single Image Done~"
echo "========================"
END=$(date +%y-%m/%d-%H/%M/%S)
echo "========================"
echo "BUILD START at $START !!"
echo "FINISH at $END !!"
echo "========================"
