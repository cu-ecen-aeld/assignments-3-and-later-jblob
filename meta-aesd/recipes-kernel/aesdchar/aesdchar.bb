SUMMARY = "AESD Char Driver"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

inherit module update-rc.d

EXTRA_OEMAKE += "modules"

SRC_URI = "git://github.com/cu-ecen-aeld/assignments-3-and-later-jblob.git;protocol=https;branch=main"
SRCREV = "6fd3449aec9ada76f104ed7d6e01ff93a3b59aed"

S = "${WORKDIR}/git/aesd-char-driver"

INITSCRIPT_NAME = "aesdchar-init"
INITSCRIPT_PARAMS = "defaults 90 10"

do_install:append() {
    # 1. Zielverzeichnisse im RootFS erstellen
    install -d ${D}${sysconfdir}/init.d
    install -d ${D}${bindir}

    # 2. Skripte direkt aus dem Source-Verzeichnis (${S}) kopieren
    install -m 0755 ${S}/aesdchar_load ${D}${bindir}/aesdchar_load
    install -m 0755 ${S}/aesdchar_unload ${D}${bindir}/aesdchar_unload
    
    # Unser neues Init-Skript kopieren
    install -m 0755 ${S}/aesdchar-init ${D}${sysconfdir}/init.d/aesdchar-init
}

FILES:${PN} += " \
    ${sysconfdir}/init.d/aesdchar-init \
    ${bindir}/aesdchar_load \
    ${bindir}/aesdchar_unload \
