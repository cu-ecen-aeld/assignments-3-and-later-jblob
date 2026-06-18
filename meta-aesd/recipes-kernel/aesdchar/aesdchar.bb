SUMMARY = "AESD Char Driver"
LICENSE = "MIT"

inherit module

SRC_URI = "git://github.com/cu-ecen-aeld/assignments-3-and-later-jblob.git;protocol=https;branch=main"

SRCREV = "79c395565f9b68dcb77c8c017244967a440e4953"

S = "${WORKDIR}/git/aesd-char-driver"
