#!/usr/bin/env bash
mkdir -p qsdk/dl
cp -rf ../src/NHSS.QSDK.4.0/apss_proc/out/proprietary/Wifi/qsdk-qca-wifi-10.4-0.1.453.245/* qsdk/
cp -rf ../src/NHSS.QSDK.4.0/apss_proc/out/proprietary/Wifi/qsdk-qca-wlan-10.4-0.1.453.245/* qsdk/
cp -rf ../src/NHSS.QSDK.4.0/apss_proc/out/proprietary/Wifi/qsdk-ieee1905-security-0.1.453.245/* qsdk/
cp -rf ../src/NHSS.QSDK.4.0/apss_proc/out/proprietary/QSDK-Base/qca-lib-0.1.453.245/* qsdk
cp -rf ../src/NHSS.QSDK.4.0/apss_proc/out/proprietary/QSDK-Base/qca-mcs-apps-0.1.453.245/* qsdk
tar -xzvf ../src/WLAN.BL.3.4/cnss_proc/src/components/qca-wifi-fw-src-component-cmn-WLAN.BL.3.4-00071-S-1.tgz
mv include qsdk/qca/src/qca-wifi-10.4/fwcommon
tar -xzvf ../src/WLAN.BL.3.4/cnss_proc/src/components/qca-wifi-fw-src-component-halphy_tools-WLAN.BL.3.4-00071-S-1.tgz
mv wlan/halphy_tools qsdk/qca/src/qca-wifi-10.4
tar xjvf ../src/NHSS.QSDK.4.0/apss_proc/out/proprietary/BLUETOPIA/qca-bluetopia-0.1.453.245.tar.bz2 -C qsdk
cp ../src/WLAN.BL.3.4/cnss_proc/bin/IPQ4019/hw.1/* qsdk/dl
cp ../src/WLAN.BL.3.4/cnss_proc/bin/QCA9888/hw.2/* qsdk/dl
cp ../src/WLAN.BL.3.4/cnss_proc/bin/QCA9984/hw.1/* qsdk/dl
cp -rf ../src/WLAN.BL.3.4/cnss_proc/src/components/* qsdk/dl
cp ../src/CNSS.PS.2.4/* qsdk/dl
cp -rf ../src/NHSS.QSDK.4.0/apss_proc/out/proprietary/Hyfi/hyfi-ipq-0.1.453.245-template/* qsdk
cp -rf ../src/NHSS.QSDK.4.0/apss_proc/out/proprietary/Wifi/qsdk-whc-0.1.453.245/* qsdk
cp -rf ../src/NHSS.QSDK.4.0/apss_proc/out/proprietary/Wifi/qsdk-whcpy-0.1.453.245/* qsdk
