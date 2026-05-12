#!/bin/bash
# Umgebung laden
source poky/oe-init-build-env build

# Wir löschen QEMU_EXTRA_ARGS, falls sie von außen kommen, 
# um Konflikte mit der local.conf zu vermeiden
export QEMU_EXTRA_ARGS=""

# Starte QEMU mit nographic.
# Das 'slirp' Kommando triggert die QB_SLIRP_OPT aus der local.conf korrekt an.
runqemu nographic slirp
