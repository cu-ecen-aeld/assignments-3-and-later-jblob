LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=8ed1a118f474eea5e159b560c339329b"

# used github Repo
SRC_URI = "git://github.com/cu-ecen-aeld/assignments-3-and-later-jblob.git;protocol=https;branch=main"
SRCREV = "0831d9b799174cc21c00393c1ec2d0f9f9d9c0fc"

S = "${WORKDIR}/git"

inherit module

# force make to work in subdirectory scull only
EXTRA_OEMAKE += "KERNELDIR=${STAGING_KERNEL_DIR} M=${S}/scull"

do_compile() {
    # Wir rufen make explizit nur für den scull-Ordner auf
    oe_runmake -C ${S}/scull
}

do_install() {
    # manuel installation of the built module
    install -d ${D}${base_libdir}/modules/${KERNEL_VERSION}/extra
    install -m 0755 ${S}/scull/scull.ko ${D}${base_libdir}/modules/${KERNEL_VERSION}/extra/
}

# load module at boot
KERNEL_MODULE_AUTOLOAD += "scull"
