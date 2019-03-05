include $(TOPDIR)/rules.mk

# Build package version from git sha-1
PKG_NAME:=luci-app-lte
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=ssh://qca-git01.qualcomm.com:29418/qsdk/qca/src/luci-app-lte
PKG_BRANCH:="master"
PKG_RELEASE:=1

JQUERY_URL="http://code.jquery.com"
JQUERY_VERSION=2.1.4

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

define Package/luci-app-lte
  SECTION:=luci
  CATEGORY:=LuCI
  TITLE:=Luci Application for LTE Connectivity
  DEPENDS:=+luci +sierra-cm
  MAINTAINER:=Earl Manning <earl.manning@qca.qualcomm.com>
  SUBMENU:=3. Applications
  MENU:=1
endef

define Package/luci-app-lte/description
Luci Application for Monitoring and Management of LTE Connectivity.
endef

define Build/Configure
endef

define Build/Compile
endef


## don't use '$(INSTALL_DATA)' because that doesn't do recursive
define Package/luci-app-lte/install
	$(INSTALL_DIR) $(1)/usr
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci
	$(CP) -aR $(PKG_BUILD_DIR)/luasrc/* $(1)/usr/lib/lua/luci

	wget "$(JQUERY_URL)/jquery-$(JQUERY_VERSION).min.js"
	$(INSTALL_DIR) $(1)/www/js
	mv jquery-$(JQUERY_VERSION).min.js $(1)/www/js/
	$(LN) jquery-$(JQUERY_VERSION).min.js $(1)/www/js/jquery-$(JQUERY_VERSION).js
endef

$(eval $(call BuildPackage,luci-app-lte))
