#!/usr/bin/env bash

cp switchbros-updater.elf release-switchbros-updater.elf

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd $DIR
rm -r switch/switchbros-updater/
mkdir -p switch/switchbros-updater/
cp switchbros-updater.nro switch/switchbros-updater/
#VERSION=$(grep "APP_VERSION :=" Makefile | cut -d' ' -f4)
#cp sbu-forwarder/sbu-forwarder.nro switch/switchbros-updater/switchbros-updater-v$VERSION.nro
zip -FSr switchbros-updater.zip switch/