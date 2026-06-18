LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=8ed1a118f474eea5e159b560c339329b"

# used github Repo and local startscript
SRC_URI = "git://github.com/cu-ecen-aeld/assignments-3-and-later-jblob.git;protocol=https;branch=main \
           file://S98lddmodules"

SRCREV = "0ee067649a9670d1e8571ef96581307af99d0d6c"
S = "${WORKDIR}/git"

inherit module update-rc.d

# Init-Script Konfiguration für Yocto
INITSCRIPT_PACKAGES = "${PN}"
INITSCRIPT_NAME:${PN} = "S98lddmodules"
INITSCRIPT_PARAMS:${PN} = "defaults 97"

# force make to work in subdirectory scull only
EXTRA_OEMAKE += "KERNELDIR=${STAGING_KERNEL_DIR} M=${S}/misc-modules"

do_compile() {
    oe_runmake -C ${S}/misc-modules
}

do_install() {
    # 1. install kernel modules (only from subdirectoy misc-modules)
    install -d ${D}${base_libdir}/modules/${KERNEL_VERSION}/extra
    install -m 0755 ${S}/misc-modules/hello.ko ${D}${base_libdir}/modules/${KERNEL_VERSION}/extra/
    install -m 0755 ${S}/misc-modules/faulty.ko ${D}${base_libdir}/modules/${KERNEL_VERSION}/extra/

    # 2. install startscript
    install -d ${D}${sysconfdir}/init.d
    install -m 0755 ${WORKDIR}/S98lddmodules ${D}${sysconfdir}/init.d/S98lddmodules
}

# enhance filelist for packet
FILES:${PN} += "${sysconfdir}/init.d/S98lddmodules"
FILES:${PN} += "${base_libdir}/modules/${KERNEL_VERSION}/extra/*.ko"

# modules are installed manually -> avoid yocto generating dependencies to kernel module 'hello' etc.
RDEPENDS:${PN} = ""
KERNEL_MODULE_AUTOLOAD = ""

# avoid error messages about installed but unpackaged files
PACKAGES = "${PN}"

# deactivate checking debug files
INSANE_SKIP:${PN} += "debug-files"
ERROR_QA:remove = "debug-files"
