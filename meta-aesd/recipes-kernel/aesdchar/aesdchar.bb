SUMMARY = "AESD Char Driver"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

inherit module

SRC_URI = "git://github.com/cu-ecen-aeld/assignments-3-and-later-jblob.git;protocol=https;branch=main"

SRCREV = "2484f14393fdc0c6ee6cd2acbc2eaf4543cc57d0"

S = "${WORKDIR}/git/aesd-char-driver"

do_install:append() {
    install -d ${D}/lib/modules/${KERNEL_VERSION}/extra
    install -m 0644 ${B}/*.ko ${D}/lib/modules/${KERNEL_VERSION}/extra
}

FILES:${PN} += "/lib/modules/${KERNEL_VERSION}/extra/aesdchar.ko"
