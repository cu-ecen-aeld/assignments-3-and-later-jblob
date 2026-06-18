SUMMARY = "AESD Char Driver"
LICENSE = "MIT"

inherit module

SRC_URI = "git://github.com/cu-ecen-aeld/assignments-3-and-later-jblob.git;protocol=https;branch=main"

SRCREV = "51e8b3b3b4c49d71e6e97c2494141010f95a52f2"

S = "${WORKDIR}/git/aesd-char-driver"
