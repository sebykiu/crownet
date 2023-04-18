#!/bin/bash
CONTAINER='omnetpp-ide'
#VERSION_TAG="latest"
VERSION_TAG="6.0.1"
# change base name to update version.
BASE_NAME="omnetpp-6.0.1"

# IDE download
URL_BASE="https://github.com/omnetpp/omnetpp/releases/download/$BASE_NAME/"
OPP_TAR="$BASE_NAME-linux-x86_64.tgz"
TMP="/tmp/$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 6 | head -n 1)"
IDE_TAR="ide.tar.gz"
CACHE="ide"

# fail fast during ide download and repackaging 
set -e
if [[ ! -d "$CACHE" ]]; then
  mkdir --verbose --parent $CACHE
fi

if [[ ! -f "$CACHE/$OPP_TAR" ]]; then
    echo "download omnet archive"
    wget "${URL_BASE}${OPP_TAR}" -O "$CACHE/$OPP_TAR"
    echo "remove old IDE archive $IDE_TAR"
    rm -f "$CACHE/$IDE_TAR"
else
    echo "use cached version: $CACHE/$OPP_TAR"
fi

if [[ ! -f "$CACHE/$IDE_TAR" ]]; then 
    echo "extract $OPP_TAR to $TMP"
    mkdir --verbose --parent  "$TMP"
    tar -xf "$CACHE/$OPP_TAR" --directory "$TMP"

    OPP_SUBFOLDER="$BASE_NAME"
    # handle special case: 6.0rc2 is stored in 6.0 folder
    if [[ "$OPP_SUBFOLDER" == "omnetpp-6.0rc2" ]]; then
        OPP_SUBFOLDER="omnetpp-6.0"
    fi

    echo "extract IDE tar from $TMP/$OPP_SUBFOLDER"
    tar -zcf "$CACHE/$IDE_TAR" -C "$TMP/$OPP_SUBFOLDER" ide
else
    echo "use cased version: $CACHE/$IDE_TAR"
fi
set +e

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
$DIR/../../scripts/upd_container.sh $CONTAINER $VERSION_TAG