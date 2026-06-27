# See https://git.yoctoproject.org/poky/tree/meta/files/common-licenses
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

# Wir nutzen den exakten, absoluten Pfad des Layers als Basis!
# '${LAYERDIR_meta-aesd}' zeigt direkt auf deinen meta-aesd Ordner.
# Von dort gehen wir ins Root-Verzeichnis und holen den server-Ordner.
SRC_URI = "file://${LAYERDIR_meta-aesd}/../server"

# Da Bitbake den Inhalt des server-Ordners flach in WORKDIR entpackt:
S = "${WORKDIR}"

# Verhindert, dass Yocto nach Git-Metadaten sucht
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
