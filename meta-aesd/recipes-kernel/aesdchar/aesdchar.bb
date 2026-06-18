SUMMARY = "AESD Char Driver"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

inherit module

SRC_URI = "git://github.com/cu-ecen-aeld/assignments-3-and-later-jblob.git;protocol=https;branch=main"

SRCREV = "25f3c748bd0c5e16e96077f82513fe14d5eff80e"

S = "${WORKDIR}/git/aesd-char-driver"

FILES:${PN} += "/lib/modules/${KERNEL_VERSION}/extra/aesdchar.ko"
