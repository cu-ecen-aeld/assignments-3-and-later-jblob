SUMMARY = "AESD Char Driver"
LICENSE = "MIT"

inherit module

SRC_URI = "git://github.com/cu-ecen-aeld/assignments-3-and-later-jblob.git;protocol=https;branch=main"

SRCREV = "6084344990914365fc3ce6ba1c175f75034b78c2"

S = "${WORKDIR}/git/aesd-char-driver"
