#!/usr/bin/env bash
# Build and install the x86_64-elf cross toolchain (binutils + gcc) for Synthax X.
# Follows the OSDev "GCC Cross-Compiler" recipe.
#
# Source order:
#   1. Local tarballs in /usr/src (Ubuntu binutils-source / gcc-NN-source packages)
#   2. ${BINUTILS_URL} / ${GCC_URL} downloaded with curl
#
# Required apt packages on Ubuntu:
#   build-essential bison flex texinfo libgmp3-dev libmpc-dev libmpfr-dev libisl-dev
#   binutils-source gcc-14-source     # if using the apt source path
#
# Usage:   sudo bash scripts/install_toolchain.sh
# Result:  ${PREFIX:-/opt/cross}/bin/x86_64-elf-{gcc,ld,as,...}
#
# After install, add to your PATH:
#   export PATH="/opt/cross/bin:$PATH"

set -euo pipefail

PREFIX="${PREFIX:-/opt/cross}"
TARGET="${TARGET:-x86_64-elf}"
BUILD_DIR="${BUILD_DIR:-/tmp/synthax-toolchain}"
JOBS="${JOBS:-$(nproc)}"

BINUTILS_VER="${BINUTILS_VER:-2.42}"
GCC_VER="${GCC_VER:-14.2.0}"

BINUTILS_URL="${BINUTILS_URL:-https://ftp.gnu.org/gnu/binutils/binutils-${BINUTILS_VER}.tar.xz}"
GCC_URL="${GCC_URL:-https://ftp.gnu.org/gnu/gcc/gcc-${GCC_VER}/gcc-${GCC_VER}.tar.xz}"

LOCAL_BINUTILS="/usr/src/binutils/binutils-${BINUTILS_VER}.tar.xz"
LOCAL_GCC="/usr/src/gcc-14/gcc-${GCC_VER}.tar.xz"

echo "== Synthax X toolchain build =="
echo "  prefix : $PREFIX"
echo "  target : $TARGET"
echo "  build  : $BUILD_DIR"
echo "  jobs   : $JOBS"
echo "  binutils: $BINUTILS_VER"
echo "  gcc     : $GCC_VER"

if command -v "${TARGET}-gcc" >/dev/null 2>&1; then
    echo "${TARGET}-gcc already on PATH at $(command -v ${TARGET}-gcc); aborting."
    exit 0
fi

mkdir -p "$BUILD_DIR" "$PREFIX"
cd "$BUILD_DIR"

fetch() {
    local local_path="$1" url="$2" out="$3"
    if [ -f "$out" ]; then
        echo "  cached: $out"
    elif [ -f "$local_path" ]; then
        echo "  copy:   $local_path"
        cp "$local_path" "$out"
    else
        echo "  fetch:  $url"
        curl -fSL --retry 4 --retry-delay 2 -o "$out" "$url"
    fi
}

echo "-- acquiring sources --"
fetch "$LOCAL_BINUTILS" "$BINUTILS_URL" "binutils-${BINUTILS_VER}.tar.xz"
fetch "$LOCAL_GCC"      "$GCC_URL"      "gcc-${GCC_VER}.tar.xz"

echo "-- extracting --"
[ -d "binutils-${BINUTILS_VER}" ] || tar xf "binutils-${BINUTILS_VER}.tar.xz"
[ -d "gcc-${GCC_VER}" ]           || tar xf "gcc-${GCC_VER}.tar.xz"

echo "-- building binutils --"
rm -rf build-binutils && mkdir build-binutils && cd build-binutils
../binutils-${BINUTILS_VER}/configure \
    --target="$TARGET" \
    --prefix="$PREFIX" \
    --with-sysroot \
    --disable-nls \
    --disable-werror
make -j"$JOBS"
make install
cd ..

export PATH="$PREFIX/bin:$PATH"

echo "-- building gcc (c only, freestanding) --"
rm -rf build-gcc && mkdir build-gcc && cd build-gcc
../gcc-${GCC_VER}/configure \
    --target="$TARGET" \
    --prefix="$PREFIX" \
    --disable-nls \
    --enable-languages=c \
    --without-headers \
    --with-system-zlib
make -j"$JOBS" all-gcc
make -j"$JOBS" all-target-libgcc
make install-gcc
make install-target-libgcc

echo "-- done --"
"$PREFIX/bin/${TARGET}-gcc" --version | head -1
"$PREFIX/bin/${TARGET}-ld"  --version | head -1
echo ""
echo "Add to your shell rc:  export PATH=\"$PREFIX/bin:\$PATH\""
