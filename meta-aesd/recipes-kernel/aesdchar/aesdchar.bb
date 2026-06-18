SUMMARY = "AESD Char Driver"
LICENSE = "MIT"

inherit module

SRC_URI = "git://github.com/cu-ecen-aeld/assignments-3-and-later-jblob.git;protocol=https;branch=main"

SRCREV = "dc4b742c8d85680698b7f3b7aab1f5ea2ec285e0"

S = "${WORKDIR}/git/aesd-char-driver"
