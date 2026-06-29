# See https://git.yoctoproject.org/poky/tree/meta/files/common-licenses

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://server"

S = "${WORKDIR}/server"

PV = "1.0"

inherit update-rc.d

INIT_SCRIPT_PACKAGES = "${PN}"
INITSCRIPT_NAME:${PN} = "S99aesdsocket"

FILES:${PN} += "${bindir}/aesdsocket"
FILES:${PN} += "${sysconfdir}/init.d/S99aesdsocket"

TARGET_LDFLAGS += "-lpthread -lrt"

do_configure () {
	:
}

do_compile () {
	oe_runmake
}

do_install () {
	install -d ${D}${bindir}
	install -m 0755 aesdsocket ${D}${bindir}/
	install -d ${D}${sysconfdir}/init.d
	
	# Nutzt den korrekten Pfad ausgehend von der CI-Verzeichnisstruktur
	install -m 0755 ${BASE_WORKDIR}/../../.. /assignment-autotest/test/assignment9-yocto/S99aesdsocket ${D}${sysconfdir}/init.d/
}
