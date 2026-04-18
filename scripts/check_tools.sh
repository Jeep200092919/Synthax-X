#!/usr/bin/env bash
# Verify the toolchain needed to build Synthax X.
set -u

TOOLS=(nasm x86_64-elf-gcc x86_64-elf-ld grub-mkrescue xorriso qemu-system-x86_64)
missing=0

echo "== Synthax X toolchain check =="
for t in "${TOOLS[@]}"; do
    if command -v "$t" >/dev/null 2>&1; then
        printf "  [OK]      %-24s -> %s\n" "$t" "$(command -v "$t")"
    else
        printf "  [MISSING] %s\n" "$t"
        missing=$((missing + 1))
    fi
done

if [ "$missing" -gt 0 ]; then
    echo ""
    echo "$missing tool(s) missing. On Debian/Ubuntu install with:"
    echo "  sudo apt install nasm grub-pc-bin grub-common xorriso qemu-system-x86"
    echo "  (and build the x86_64-elf-gcc cross-compiler per OSDev wiki)"
    exit 1
fi

echo "All required tools are present."
