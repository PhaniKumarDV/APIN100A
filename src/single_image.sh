#!/usr/bin/env bash

if [ "$1" = "std" ]; then
	echo "ipq40xx_standard configuration !!"
	cp ../sw/meta-scripts/ipq40xx_standard/* IPQ4019.ILQ.4.0/common/build/ipq
else
	echo "ipq40xx_premium configuration !!"
	cp ../sw/meta-scripts/ipq40xx_premium/* IPQ4019.ILQ.4.0/common/build/ipq
fi

cp -rf ../sw/qsdk/qca/src/uboot-1.0/tools/pack.py NHSS.QSDK.4.0/apss_proc/out/
cp -rf TZ.BF.2.7/trustzone_images/build/ms/bin/MAZAANAA/* IPQ4019.ILQ.4.0/common/build/ipq
cp -rf ../sw/qsdk/bin/ipq806x/openwrt* IPQ4019.ILQ.4.0/common/build/ipq

if [ "$1" = "std" ]; then
	echo "ipq40xx_standard configuration !!"
	cp BOOT.BF.3.1.1/boot_images/build/ms/bin/40xx/misc/tools/config/boardconfig_standard IPQ4019.ILQ.4.0/common/build/ipq
	cp BOOT.BF.3.1.1/boot_images/build/ms/bin/40xx/misc/tools/config/appsboardconfig_standard IPQ4019.ILQ.4.0/common/build/ipq
else
	echo "ipq40xx_premium configuration !!"
	cp BOOT.BF.3.1.1/boot_images/build/ms/bin/40xx/misc/tools/config/boardconfig_premium IPQ4019.ILQ.4.0/common/build/ipq
	cp BOOT.BF.3.1.1/boot_images/build/ms/bin/40xx/misc/tools/config/appsboardconfig_premium IPQ4019.ILQ.4.0/common/build/ipq
fi

sed -i 's#</linux_root_path>#/</linux_root_path>#' IPQ4019.ILQ.4.0/contents.xml
sed -i 's#</windows_root_path>#\\</windows_root_path>#' IPQ4019.ILQ.4.0/contents.xml

cd IPQ4019.ILQ.4.0/common/build



if [ "$1" = "std" ]; then
	echo "ipq40xx_standard configuration !!"
	sed '/debug/d' -i update_common_info_standard.py
	sed '/s-gcc5/d' -i update_common_info_standard.py
	python update_common_info_standard.py
else
	echo "ipq40xx_premium configuration !!"
	sed '/debug/d' -i update_common_info.py
	sed /Required/d -i update_common_info.py
	sed /streamboost/d -i update_common_info.py
	python update_common_info.py
fi 
