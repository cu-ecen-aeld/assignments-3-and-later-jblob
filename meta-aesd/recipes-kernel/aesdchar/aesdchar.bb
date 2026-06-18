SUMMARY = "AESD Char Driver"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

inherit module

SRC_URI = "git://github.com/cu-ecen-aeld/assignments-3-and-later-jblob.git;protocol=https;branch=main"

SRCREV = "0831d9b799174cc21c00393c1ec2d0f9f9d9c0fc"

S = "${WORKDIR}/git/aesd-char-driver"

FILES:${PN} += "/lib/modules/${KERNEL_VERSION}/extra/aesdchar.ko"
