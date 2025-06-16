#!/usr/bin/env sh

UUID=$(uuidgen)
TMPDIR="/tmp/TMP-${UUID}/"
git clone https://github.com/floooh/sokol-tools-bin.git ${TMPDIR}
mv ${TMPDIR}/bin .
rm -rf ${TMPDIR}
