
##############################################################
#
# AESD-ASSIGNMENTS
#
##############################################################

#TODO: Fill up the contents below in order to reference your assignment 3 git contents
AESD_ASSIGNMENTS_VERSION = 1d6033d5d5c665f518f3a33f919dc8fcb8e7f422
###AESD_ASSIGNMENTS_VERSION = main
# Note: Be sure to reference the *ssh* repository URL here (not https) to work properly
# with ssh keys and the automated build/test system.
# Your site should start with git@github.com:
AESD_ASSIGNMENTS_SITE = git@github.com:cu-ecen-aeld/assignments-3-and-later-jblob.git
# AESD_ASSIGNMENTS_SITE = https://github.com/cu-ecen-aeld/assignments-3-and-later-jblob.git
AESD_ASSIGNMENTS_SITE_METHOD = git
AESD_ASSIGNMENTS_GIT_SUBMODULES = YES

define AESD_ASSIGNMENTS_BUILD_CMDS
	$(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(@D)/finder-app all
	$(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(@D)/server all
	# removes hidden Windows line endings
	sed -i 's/\r$$//' $(@D)/server/aesdsocket-start-stop
endef

# TODO add your writer, finder and finder-test utilities/scripts to the installation steps below
define AESD_ASSIGNMENTS_INSTALL_TARGET_CMDS
	# 1. Config locations
	$(INSTALL) -d -m 0755 $(TARGET_DIR)/etc/finder-app/conf/
	$(INSTALL) -m 0644 $(@D)/conf/* $(TARGET_DIR)/etc/finder-app/conf/
	$(INSTALL) -d -m 0755 $(TARGET_DIR)/usr/bin/conf/
	$(INSTALL) -m 0644 $(@D)/conf/* $(TARGET_DIR)/usr/bin/conf/

	# 2. Binaries and Scripts
	$(INSTALL) -m 0755 $(@D)/finder-app/writer $(TARGET_DIR)/usr/bin
	$(INSTALL) -m 0755 $(@D)/finder-app/finder.sh $(TARGET_DIR)/usr/bin
	$(INSTALL) -m 0755 $(@D)/finder-app/finder-test.sh $(TARGET_DIR)/usr/bin
	
	# 3. The Helpers & Test Script (BASED ON YOUR FIND RESULTS)
	# Note: No .sh extension on script-helpers
	$(INSTALL) -m 0755 $(@D)/assignment-autotest/test/shared/script-helpers $(TARGET_DIR)/usr/bin/script-helpers.sh
	$(INSTALL) -m 0755 $(@D)/assignment-autotest/test/assignment1/assignment-1-test.sh $(TARGET_DIR)/usr/bin/
endef

$(eval $(generic-package))
