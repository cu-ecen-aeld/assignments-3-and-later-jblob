SUMMARY = "AESD Char Driver"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

inherit module

EXTRA_OEMAKE += "modules"

SRC_URI = "git://github.com/cu-ecen-aeld/assignments-3-and-later-jblob.git;protocol=https;branch=main"
SRCREV = "b10968b3431ff59b2a2b1a84a055fea4f8c51c9e"

S = "${WORKDIR}/git/aesd-char-driver"

#  MANUELL installieren (robust!)
do_install:append() {
    install -d ${D}/lib/modules/${KERNEL_VERSION}/extra
    install -m 0644 ${B}/*.ko ${D}/lib/modules/${KERNEL_VERSION}/extra
}

#  KEIN kernel-module Paket mehr verwenden!
FILES:${PN} += "/lib/modules/${KERNEL_VERSION}/extra/aesdchar.ko"
