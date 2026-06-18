SUMMARY = "AESD Char Driver"
LICENSE = "MIT"

inherit module

SRC_URI = "git://github.com/cu-ecen-aeld/assignments-3-and-later-jblob.git;protocol=https;branch=main"

SRCREV = "7e35e7a8aff17e20aeaa8212ff3d57e67ba5d794"

S = "${WORKDIR}/git/aesd-char-driver"
