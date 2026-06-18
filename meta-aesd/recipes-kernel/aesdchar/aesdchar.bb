SUMMARY = "AESD Char Driver"
LICENSE = "MIT"

inherit module

SRC_URI = "git://github.com/cu-ecen-aeld/assignments-3-and-later-jblob.git;protocol=https;branch=main"

SRCREV = "461b7f9c132bc9ce459fc2c03a7b305a2359616f"

S = "${WORKDIR}/git/aesd-char-driver"
