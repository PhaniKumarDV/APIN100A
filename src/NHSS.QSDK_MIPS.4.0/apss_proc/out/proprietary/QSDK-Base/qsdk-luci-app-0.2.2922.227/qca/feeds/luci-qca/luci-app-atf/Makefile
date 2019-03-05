include $(TOPDIR)/rules.mk

# Build package version from git sha-1
PKG_NAME:=luci-app-atf
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=ssh://qca-git01.qualcomm.com:29418/qsdk/qca/src/luci-app-atf
PKG_BRANCH:="dev/1.0"
PKG_RELEASE:=1

include $(INCLUDE_DIR)/local-development.mk
ifeq ($(DUMP)$(PKG_VERSION),)
  PKG_REV:=$(shell git ls-remote $(PKG_SOURCE_URL) $(PKG_BRANCH) | cut -b -7)
  PKG_VERSION:=g$(PKG_REV)
endif
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=$(PKG_REV)
PKG_SOURCE_MIRROR:=none

include $(INCLUDE_DIR)/package-version-override.mk
include $(INCLUDE_DIR)/package.mk

define Package/luci-app-atf
  SECTION:=luci
  CATEGORY:=LuCI
  TITLE:=Luci Application for Wifi Airtime Fairness Features
  DEPENDS:=+luci
  MAINTAINER:=Earl Manning <earl.manning@qca.qualcomm.com>
  SUBMENU:=3. Applications
  MENU:=1
endef

define Package/luci-app-atf/description
Luci Application for Monitoring and Management of Wifi's Airtime Fairness Features
endef

define Build/Configure
endef

define Build/Compile
endef


## don't use '$(INSTALL_DATA)' because that doesn't do recursive
define Package/luci-app-atf/install
	$(INSTALL_DIR) $(1)/usr
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci
	$(CP) -aR $(PKG_BUILD_DIR)/luasrc/* $(1)/usr/lib/lua/luci

	$(INSTALL_DIR) $(1)/www/js
	$(CP) $(PKG_BUILD_DIR)/htdocs/js/* $(1)/www/js

	$(INSTALL_DIR) $(1)/www/luci-static
	$(INSTALL_DIR) $(1)/www/luci-static/resources
	$(INSTALL_DIR) $(1)/www/luci-static/resources/darmok
	$(INSTALL_DIR) $(1)/www/luci-static/resources/darmok/images
	$(CP) -aR $(PKG_BUILD_DIR)/htdocs/luci-static/resources/darmok/images/* \
			$(1)/www/luci-static/resources/darmok/images
	cd $(1)/www/; ln -s luci-static/resources/darmok/images images
endef

$(eval $(call BuildPackage,luci-app-atf))