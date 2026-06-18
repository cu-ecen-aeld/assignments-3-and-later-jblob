SUMMARY = "AESD Char Driver"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

inherit module

EXTRA_OEMAKE += "modules"

SRC_URI = "git://github.com/cu-ecen-aeld/assignments-3-and-later-jblob.git;protocol=https;branch=main"
SRCREV = "5a72e17749d989da6112baee49684fb214c3f7d9"

S = "${WORKDIR}/git/aesd-char-driver"
KERNEL_MODULE_AUTOLOAD += "aesdchar"
