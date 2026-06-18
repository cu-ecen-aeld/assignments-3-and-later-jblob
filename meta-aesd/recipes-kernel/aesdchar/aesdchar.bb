SUMMARY = "AESD Char Driver"
LICENSE = "MIT"

inherit module

SRC_URI = "git://github.com/cu-ecen-aeld/assignments-3-and-later-jblob.git;protocol=https;branch=main"

SRCREV = "5cdade01e196546f8ccf5eabbca3ba1dafe00df6"

S = "${WORKDIR}/git/aesd-char-driver"
