SUMMARY = "AESD Char Driver"
LICENSE = "MIT"

inherit module

SRC_URI = "git://github.com/cu-ecen-aeld/assignments-3-and-later-jblob.git;protocol=https;branch=main"

SRCREV = "4435e1145419c55bf826b09d73519e442074a5c1"

S = "${WORKDIR}/git/aesd-char-driver"
