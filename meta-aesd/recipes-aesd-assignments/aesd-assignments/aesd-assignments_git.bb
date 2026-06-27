# See https://git.yoctoproject.org/poky/tree/meta/files/common-licenses
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

# TODO: Set this  with the path to your assignments rep.  Use ssh protocol and see lecture notes
# about how to setup ssh-agent for passwordless access
####SRC_URI = "git://git@github.com/cu-ecen-aeld/assignments-3-and-later-jblob.git;protocol=ssh;branch=main"
####SRC_URI = "git://github.com/cu-ecen-aeld/assignments-3-and-later-jblob.git;protocol=ssh;branch=main"
####SRC_URI = "ssh://git@github.com/cu-ecen-aeld/assignments-3-and-later-jblob.git;protocol=ssh;branch=main"
## ## ### SRC_URI = "git://github.com/cu-ecen-aeld/assignments-3-and-later-jblob.git;protocol=https;branch=main"
## ## ###SRC_URI = "git://github.com/cu-ecen-aeld/assignments-3-and-later-jblob.git;branch=main;protocol=https"
#SRC_URI = "https://github.com/cu-ecen-aeld/assignments-3-and-later-jblob.git;branch=main"

#SRC_URI = "git://github.com/cu-ecen-aeld/assignments-3-and-later-jblob.git;branch=main;protocol=https"
#SRC_URI = "file://${FILE_DIRNAME}/../../../../"
#SRC_URI = "git://github.com/cu-ecen-aeld/assignments-3-and-later-jblob.git;protocol=https;branch=main"
#SRC_URI = "git://${FILE_DIRNAME}/../../../../;protocol=file;branch=main"
#SRC_URI = "file://${FILE_DIRNAME}/../../../../"
#SRC_URI = "file://${TOPDIR}/../assignments-3-and-later-jblob"
SRC_URI = "file://server"
# Wir erweitern den Suchpfad für Bitbake, damit es den Ordner findet
FILESEXTRAPATHS:prepend := "${THISDIR}/../../../../:"
PV = "1.0+git${SRCPV}"
# TODO: set to reference a specific commit hash in your assignment repo
SRCREV = "358fbc17f0032de01b63f71799d58d7f655461c1"

#####SRCREV = "${AUTOREV}"



# This sets your staging directory based on WORKDIR, where WORKDIR is defined at 
# https://docs.yoctoproject.org/ref-manual/variables.html?highlight=workdir#term-WORKDIR
# We reference the "server" directory here to build from the "server" directory
# in your assignments repo
#S = "${WORKDIR}/git/server"
#S = "${WORKDIR}/server"
#S = "${WORKDIR}"
# Da Bitbake den Inhalt flach in WORKDIR entpackt,
# setzen wir S direkt auf WORKDIR:
S = "${WORKDIR}"
#B = "${S}/server"
#S = "${WORKDIR}/assignments-3-and-later-jblob/server"

# Verhindert, dass Yocto nach Git-Metadaten sucht, die bei file:// nicht existieren:
BB_STRICT_CHECKSUM = "0"
do_deploy_source_date_epoch[noexec] = "1"

# Verhindert, dass Yocto nach Git-Metadaten sucht, die bei file:// nicht existieren:
BB_STRICT_CHECKSUM = "0"
do_deploy_source_date_epoch[noexec] = "1"

# TODO: Add the aesdsocket application and any other files you need to install
# See https://git.yoctoproject.org/poky/plain/meta/conf/bitbake.conf?h=kirkstone
# --- INIT SCRIPT ---
inherit update-rc.d
INITSCRIPT_PACKAGES = "${PN}"
INITSCRIPT_NAME:${PN} = "S99aesdsocket"
# /--- INIT SCRIPT ---

FILES:${PN} += "${bindir}/aesdsocket"
# TODO: customize these as necessary for any libraries you need for your application
# (and remove comment)
# Add the init script to the package files list
FILES:${PN} += "${sysconfdir}/init.d/S99aesdsocket"
TARGET_LDFLAGS += "-lpthread -lrt"

do_configure () {
	:
}

do_compile () {
	oe_runmake
}

do_install () {
	# TODO: Install your binaries/scripts here.
	# Be sure to install the target directory with install -d first
	# Yocto variables ${D} and ${S} are useful here, which you can read about at 
	# https://docs.yoctoproject.org/ref-manual/variables.html?highlight=workdir#term-D
	# and
	# https://docs.yoctoproject.org/ref-manual/variables.html?highlight=workdir#term-S
	# See example at https://github.com/cu-ecen-aeld/ecen5013-yocto/blob/ecen5013-hello-world/meta-ecen5013/recipes-ecen5013/ecen5013-hello-world/ecen5013-hello-world_git.bb
	
	# Create the destination directories in the destination folder (${D})
    # ${bindir} points to /usr/bin
    # ${sysconfdir} points to /etc
    install -d ${D}${bindir}
    install -d ${D}${sysconfdir}/init.d
	install -m 0755 ${S}/aesdsocket ${D}${bindir}
    install -m 0755 ${S}/aesdsocket-start-stop ${D}${sysconfdir}/init.d/S99aesdsocket
}
