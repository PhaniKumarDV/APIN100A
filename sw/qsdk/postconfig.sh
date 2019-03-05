#!/usr/bin/env bash
for pkg_num in 2 9;do sed 's/CONFIG_PACKAGE_qca-wifi-fw-hw'${pkg_num}'-10.4-asic=y/# CONFIG_PACKAGE_qca-wifi-fw-hw'${pkg_num}'-10.4-asic is not set/g' -i .config;done
sed 's/CONFIG_PACKAGE_kmod-wil6210=y/# CONFIG_PACKAGE_kmod-wil6210 is not set/g' -i .config
sed 's/CONFIG_PACKAGE_wigig-firmware=y/# CONFIG_PACKAGE_wigig-firmware is not set/g' -i .config

#Run the following commands if the image is being built without CSRMesh support infrastructure:
sed 's/CONFIG_PACKAGE_libCsrCrypto=m/# CONFIG_PACKAGE_libCsrCrypto is not set/g' -i .config
sed 's/CONFIG_PACKAGE_csrLotApp=m/# CONFIG_PACKAGE_csrLotApp is not set/g' -i .config
sed 's/CONFIG_PACKAGE_csrMeshGw=m/# CONFIG_PACKAGE_csrMeshGw is not set/g' -i .config
sed 's/ CONFIG_PACKAGE_csrMeshGwRefApp=m/# CONFIG_PACKAGE_csrMeshGwRefApp is not set/g' -i .config
sed 's/ CONFIG_PACKAGE_csrMeshGwTestApp=m/# CONFIG_PACKAGE_csrMeshGwTestApp is not set/g' -i .config

#Use the following code to build GCC v5.2:
#sed 's/# CONFIG_TOOLCHAINOPTS is not set/CONFIG_TOOLCHAINOPTS=y/g' -i .config
#sed 's/CONFIG_GCC_VERSION_4_6_LINARO=y/# CONFIG_GCC_VERSION_4_6_LINARO is not set/g' -i .config
#sed 's/CONFIG_GCC_DEFAULT_VERSION_4_6_LINARO=y/# CONFIG_GCC_DEFAULT_VERSION_4_6_LINARO is not set/g' -i .config
#echo "CONFIG_GCC_USE_VERSION_5=y" >> .config
#sed 's/CONFIG_GCC_VERSION="4.6-linaro"/CONFIG_GCC_VERSION="5.2.0"/g' -i .config
#sed 's/CONFIG_GCC_VERSION_4_6=y/CONFIG_GCC_VERSION_5=y/g' -i .config

