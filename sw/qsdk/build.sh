#!/usr/bin/env bash
START=$(date +%y-%m/%d-%H/%M/%S)
./scripts/feeds update -a
./scripts/feeds install -a -f

if [ "$1" = "std" ]; then
	echo "ipq806x_standard configuration !!"
	cp qca/configs/qsdk/ipq806x_standard.config .config
	sed -i '/REV_PREFIX:=/ c\REV_PREFIX:=STD-' include/version.mk
else
	echo "ipq806x_premium configuration !!"
	cp qca/configs/qsdk/ipq806x_premium.config .config
	sed -i '/REV_PREFIX:=/ c\REV_PREFIX:=' include/version.mk
fi

make defconfig
./postconfig.sh
NOW=$(date +%y%m%d-%H%M%S)

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

