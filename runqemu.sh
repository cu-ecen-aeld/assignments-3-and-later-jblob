#!/bin/bash

source poky/oe-init-build-env build

# Suche die qemuboot.conf, die der Build gerade erzeugt hat
QEMUCONF=$(ls -t /build/tmp-glibc/deploy/images/qemuarm64/*.qemuboot.conf 2>/dev/null | head -n 1)

if [ -n "$QEMUCONF" ]; then
    echo "Using qemuboot config: $QEMUCONF"
    echo "Before patch:"
    grep hostfwd "$QEMUCONF" || true

    # Zusätzlichen Forward für aesdsocket einbauen:
    # Host localhost:9000 -> QEMU target localhost:9000
    sed -i 's/hostfwd=tcp::10022-:22/hostfwd=tcp::10022-:22,hostfwd=tcp::9000-:9000/g' "$QEMUCONF"

    echo "After patch:"
    grep hostfwd "$QEMUCONF" || true
else
    echo "WARNING: qemuboot.conf not found before runqemu"
fi

runqemu nographic slirp
