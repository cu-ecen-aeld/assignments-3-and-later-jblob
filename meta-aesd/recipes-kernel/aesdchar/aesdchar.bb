SUMMARY = "AESD Char Driver"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

inherit module

EXTRA_OEMAKE += "modules"

SRC_URI = "git://github.com/cu-ecen-aeld/assignments-3-and-later-jblob.git;protocol=https;branch=main"
SRCREV = "62f60adc75361ea2cea7cae2bf89d4ecb85ad0a4"

S = "${WORKDIR}/git/aesd-char-driver"
KERNEL_MODULE_AUTOLOAD += "aesdchar"
