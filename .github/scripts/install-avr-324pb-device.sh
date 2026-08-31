#!/usr/bin/env bash
# Install ATmega324PB device files from BoardComputer/utils into the host avr toolchain.
# Matches the manual setup described in BoardComputer/doc/build.md.
set -euo pipefail

UTILS_DIR="${1:-BoardComputer/utils}"
ZIP="${UTILS_DIR}/device-specs.zip"

if [[ ! -f "${ZIP}" ]]; then
  echo "error: ${ZIP} not found" >&2
  exit 1
fi

WORK=$(mktemp -d)
trap 'rm -rf "${WORK}"' EXIT
unzip -q "${ZIP}" -d "${WORK}"

SRC="${WORK}/device-specs"

SPECS_FILE=$(avr-gcc -print-file-name=device-specs/specs-atmega328p)
if [[ "${SPECS_FILE}" != /* ]]; then
  GCC_INSTALL=$(avr-gcc -print-search-dirs | sed -n 's/^install: //p' | tr -d ' ')
  SPECS_FILE="${GCC_INSTALL}device-specs/specs-atmega328p"
fi
DEVICE_SPECS_DIR=$(dirname "${SPECS_FILE}")

LIBC_A=$(realpath "$(avr-gcc -print-file-name=libc.a)")
AVR_LIB_ROOT=$(dirname "${LIBC_A}")
AVR_ROOT=$(dirname "${AVR_LIB_ROOT}")
AVR_INCLUDE="${AVR_ROOT}/include/avr"
AVR_LIB="${AVR_LIB_ROOT}/avr5"

for dir in "${DEVICE_SPECS_DIR}" "${AVR_INCLUDE}" "${AVR_LIB}"; do
  if [[ ! -d "${dir}" ]]; then
    echo "error: expected directory missing: ${dir}" >&2
    exit 1
  fi
done

cp "${SRC}/specs-atmega324pb" "${DEVICE_SPECS_DIR}/"
cp "${SRC}/iom324pb.h" "${AVR_INCLUDE}/"
cp "${SRC}/crtatmega324pb.o" "${SRC}/libatmega324pb.a" "${AVR_LIB}/"

echo "Installed ATmega324PB device files:"
echo "  specs -> ${DEVICE_SPECS_DIR}/specs-atmega324pb"
echo "  header -> ${AVR_INCLUDE}/iom324pb.h"
echo "  libs -> ${AVR_LIB}/crtatmega324pb.o, libatmega324pb.a"
