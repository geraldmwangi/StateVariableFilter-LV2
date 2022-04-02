######################################
#
# StateVariableFilter-LV2
#
######################################

# where to find the source code - locally in this case
STATEVARIABLEFILTER_LV2_SITE_METHOD = local
STATEVARIABLEFILTER_LV2_SITE = $($(PKG)_PKGDIR)/

# even though this is a local build, we still need a version number
# bump this number if you need to force a rebuild
STATEVARIABLEFILTER_LV2_VERSION = 1

# dependencies (list of other buildroot packages, separated by space)
# on this package we need to depend on the host version of ourselves to be able to run the ttl generator
STATEVARIABLEFILTER_LV2_DEPENDENCIES = host-StateVariableFilter-LV2

# LV2 bundles that this package generates (space separated list)
STATEVARIABLEFILTER_LV2_BUNDLES = statevf.lv2

# call make with the current arguments and path. "$(@D)" is the build directory.
STATEVARIABLEFILTER_LV2_HOST_MAKE   = $(HOST_MAKE_ENV)   $(HOST_CONFIGURE_OPTS) LV2DIR=$(@D)/bin  $(MAKE) -C $(@D) install
STATEVARIABLEFILTER_LV2_TARGET_MAKE = $(TARGET_MAKE_ENV) $(TARGET_CONFIGURE_OPTS) LV2DIR=$(@D)/bin  $(MAKE) -C $(@D) install

STATEVARIABLEFILTER_LV2_TARGET_MAKE_INSTALL = $(TARGET_MAKE_ENV) $(TARGET_CONFIGURE_OPTS) LV2DIR=$(TARGET_DIR)/usr/lib/lv2/  $(MAKE) -C $(@D) install

# temp dir where we place the generated ttls
STATEVARIABLEFILTER_LV2_TMP_DIR = $(HOST_DIR)/tmp-StateVariableFilter-LV2


# build plugins in host to generate ttls
define HOST_STATEVARIABLEFILTER_LV2_BUILD_CMDS
	# build everything
	$(STATEVARIABLEFILTER_LV2_HOST_MAKE)

	# delete binaries
	rm $(@D)/bin/*.lv2/*.so

	# create temp dir
	rm -rf $(STATEVARIABLEFILTER_LV2_TMP_DIR)
	mkdir -p $(STATEVARIABLEFILTER_LV2_TMP_DIR)

	# copy the generated bundles without binaries to temp dir
	cp -r $(@D)/bin/*.lv2 $(STATEVARIABLEFILTER_LV2_TMP_DIR)
endef

# build plugins in target skipping ttl generation
define STATEVARIABLEFILTER_LV2_BUILD_CMDS

	# copy previously generated bundles
	cp -r $(STATEVARIABLEFILTER_LV2_TMP_DIR)/*.lv2 $(@D)/bin/

	# now build in target
	$(STATEVARIABLEFILTER_LV2_TARGET_MAKE)

	# cleanup
	rm -r $(STATEVARIABLEFILTER_LV2_TMP_DIR)
endef

# install command
define STATEVARIABLEFILTER_LV2_INSTALL_TARGET_CMDS
	echo Installing
	$(STATEVARIABLEFILTER_LV2_TARGET_MAKE_INSTALL) 
endef


# import everything else from the buildroot generic package
$(eval $(generic-package))
# import host version too
$(eval $(host-generic-package))
