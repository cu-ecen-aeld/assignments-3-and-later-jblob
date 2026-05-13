LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=8ed1a118f474eea5e159b560c339329b"

# GitHub Repo und lokales Startscript aus dem 'files' Verzeichnis
SRC_URI = "git://github.com/cu-ecen-aeld/assignments-3-and-later-jblob.git;protocol=https;branch=main \
           file://S98lddmodules"

SRCREV = "${AUTOREV}"
S = "${WORKDIR}/git"

inherit module update-rc.d

# Init-Script Konfiguration für Yocto
INITSCRIPT_PACKAGES = "${PN}"
INITSCRIPT_NAME:${PN} = "S98lddmodules"
INITSCRIPT_PARAMS:${PN} = "defaults 98"

# Wir kompilieren nur im Unterordner misc-modules
EXTRA_OEMAKE += "KERNELDIR=${STAGING_KERNEL_DIR} M=${S}/misc-modules"

do_compile() {
    oe_runmake -C ${S}/misc-modules
}

do_install() {
    # 1. Kernel-Module installieren
    # Wir installieren nur die Module, die in diesem Unterordner (misc-modules) gebaut wurden.
    install -d ${D}${base_libdir}/modules/${KERNEL_VERSION}/extra
    install -m 0755 ${S}/misc-modules/hello.ko ${D}${base_libdir}/modules/${KERNEL_VERSION}/extra/
    install -m 0755 ${S}/misc-modules/faulty.ko ${D}${base_libdir}/modules/${KERNEL_VERSION}/extra/

    # 2. Startscript installieren
    install -d ${D}${sysconfdir}/init.d
    install -m 0755 ${WORKDIR}/S98lddmodules ${D}${sysconfdir}/init.d/S98lddmodules
}

# Dateiliste für das Paket erweitern
FILES:${PN} += "${sysconfdir}/init.d/S98lddmodules"
FILES:${PN} += "${base_libdir}/modules/${KERNEL_VERSION}/extra/*.ko"

# WICHTIG: Da wir die Module manuell installieren, müssen wir verhindern,
# dass Yocto automatische Abhängigkeiten zu kernel-module-hello etc. erzeugt.
RDEPENDS:${PN} = ""
KERNEL_MODULE_AUTOLOAD = ""

# Verhindert Fehlermeldungen wegen installierter aber nicht paketierter Dateien
PACKAGES = "${PN}"

# QA Issue Fix: Deaktiviert die Prüfung auf Debug-Dateien. 
# Da das System automatisch Pakete wie kernel-module-faulty erzeugt, 
# wenden wir den Skip auf alle Pakete an (speziell PN).
INSANE_SKIP:${PN} += "debug-files"

# Da die Fehler spezifisch in den automatisch generierten Kernel-Modul-Paketen
# auftreten, stellen wir sicher, dass die QA-Prüfung hier globaler greift.
ERROR_QA:remove = "debug-files"
