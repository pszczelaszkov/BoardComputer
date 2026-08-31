#!/usr/bin/env bash
# Install a recent AVR GNU toolchain (modm-io build) for CI parity with modern distros.
set -euo pipefail

VERSION="${AVR_GCC_VERSION:-14.2.0}"
INSTALL_DIR="${AVR_GCC_INSTALL_DIR:-${HOME}/avr-gcc}"
MARKER="${INSTALL_DIR}/.installed-version"
URL="https://github.com/modm-io/avr-gcc/releases/download/v${VERSION}/modm-avr-gcc.tar.bz2"

if [[ -x "${INSTALL_DIR}/bin/avr-gcc" && -f "${MARKER}" && "$(cat "${MARKER}")" == "${VERSION}" ]]; then
  echo "Using cached modm-avr-gcc ${VERSION} at ${INSTALL_DIR}"
  "${INSTALL_DIR}/bin/avr-gcc" -dumpversion
  exit 0
fi

TMP=$(mktemp -d)
trap 'rm -rf "${TMP}"' EXIT

echo "Downloading modm-avr-gcc ${VERSION}..."
curl -fsSL "${URL}" -o "${TMP}/modm-avr-gcc.tar.bz2"

PARENT=$(dirname "${INSTALL_DIR}")
mkdir -p "${PARENT}"
rm -rf "${INSTALL_DIR}"
tar xf "${TMP}/modm-avr-gcc.tar.bz2" -C "${PARENT}"
echo "${VERSION}" > "${INSTALL_DIR}/.installed-version"

echo "Installed modm-avr-gcc ${VERSION}:"
"${INSTALL_DIR}/bin/avr-gcc" -dumpversion
