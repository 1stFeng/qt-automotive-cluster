SUMMARY = "cluster"
DESCRIPTION = "the script for starting cluster"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://COPYING;md5=3da9cfbcb788c80a0384361b4de20420"

SRC_URI = "file://COPYING \
           file://mosi-cluster \
           file://Fonts \
           file://cluster.sh \
           file://settings.xml \
           file://style.conf \
           file://lib \
           file://fdbus \
"
inherit update-rc.d

S = "${WORKDIR}"

INITSCRIPT_PACKAGES = "${PN}"
INITSCRIPT_NAME = "cluster.sh"
INITSCRIPT_PARAMS_${PN} = "defaults 04"
INSANE_SKIP_${PN} += "ldflags"
INSANE_SKIP_${PN} += "installed-vs-shipped"
INSANE_SKIP_${PN} += "file-rdeps"
INSANE_SKIP_${PN} += "host-user-contaminated"
INSANE_SKIP_${PN} += "already-stripped dev-so dev-deps dev-elf"
INSANE_SKIP_${PN} += "build-deps"
INSANE_SKIP_${PN}-dev += "dev-elf file-rdeps"
FILESEXTRAPATHS_prepend := "${THISDIR}/files:"
DEPENDS = "pvr-lib messagecenter"
RDEPENDS_${PN} = "sdcast  gstreamer1.0 gstreamer1.0-libav gstreamer1.0-plugins-base gstreamer1.0-plugins-good \
                  gstreamer1.0-plugins-bad gstreamer1.0-omx libomxil gstreamer1.0-libav "

do_install () {
	install -d ${D}/${nonarch_libdir}
	cp -rf  lib/* ${D}/${nonarch_libdir}
	install -d ${D}${sysconfdir}/init.d
	install -m 0755 cluster.sh ${D}${sysconfdir}/init.d
	install -d ${D}${bindir}
	install -m 0755 mosi-cluster ${D}${bindir}/
	install -m 0755 style.conf ${D}${bindir}/
	install -m 0755 settings.xml ${D}${bindir}/
	cp -rf  fdbus/* ${D}/${bindir}/
	install -m 0755 fdbus/name_server ${D}${bindir}/
	install -m 0755 fdbus/lssvc ${D}${bindir}/
	install -d ${D}${bindir}/Fonts/
	cp -rf Fonts/* ${D}${bindir}/Fonts/
}

FILES_${PN} += "${nonarch_libdir}/* ${bindir} ${bindir}/Fonts/*"


