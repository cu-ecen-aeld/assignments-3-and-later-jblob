#!/bin/bash

# Start Yocto QEMU correctly
source poky/oe-init-build-env
runqemu nographic slirp hostfwd=tcp::10022-:22
#runqemu qemux86-64 nographic slirp
