#!/bin/bash
set -e

# ---------- CONFIG ----------
PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SRC="$PROJECT_ROOT/src"
BUILD="$PROJECT_ROOT/build"
ISO="$PROJECT_ROOT/iso_root"
ISO_NAME="myos.iso"

CC=i686-elf-gcc
LD=i686-elf-ld
AS=i686-elf-as

CFLAGS="-std=gnu99 -ffreestanding -O2 -Wall -Wextra"
LDFLAGS="-T $SRC/arch/x86/linker.ld -nostdlib"

# ---------- PREP ----------
echo "[*] Cleaning build directory..."
rm -rf "$BUILD"
mkdir -p "$BUILD"

# ---------- BUILD ASM ----------
echo "[*] Assembling..."
$AS $SRC/arch/x86/boot.s -o $BUILD/boot.o
$AS $SRC/arch/x86/gdt.S -o $BUILD/gdt_asm.o
$AS $SRC/arch/x86/idt.S -o $BUILD/idt_asm.o
$AS $SRC/arch/x86/interrept.S -o $BUILD/interrept.o

# ---------- BUILD C ----------
echo "[*] Compiling C files..."
$CC $CFLAGS -I$SRC/include -c $SRC/arch/x86/gdt.c -o $BUILD/gdt.o
$CC $CFLAGS -I$SRC/include -c $SRC/arch/x86/idt.c -o $BUILD/idt.o
$CC $CFLAGS -I$SRC/include -c $SRC/arch/x86/paging.c -o $BUILD/paging.o
$CC $CFLAGS -I$SRC/include -c $SRC/arch/x86/pmm.c -o $BUILD/pmm.o
$CC $CFLAGS -I$SRC/include -c $SRC/arch/x86/vmm.c -o $BUILD/vmm.o
$CC $CFLAGS -I$SRC/include -c $SRC/lib/kstring.c -o $BUILD/kstring.o
$CC $CFLAGS -I$SRC/include -c $SRC/kernel/kernel.c -o $BUILD/kernel.o
$CC $CFLAGS -I$SRC/include -c $SRC/arch/x86/higher_half.c -o $BUILD/higher_half.o
$CC $CFLAGS -I$SRC/include -c $SRC/arch/x86/higher_half_jump.c -o $BUILD/higher_half_jump.o
$CC $CFLAGS -I$SRC/include -c $SRC/drivers/framebuffer.c -o $BUILD/framebuffer.o
$CC $CFLAGS -I$SRC/include -c $SRC/drivers/keyboard.c -o $BUILD/keyboard.o
$CC $CFLAGS -I$SRC/include -c $SRC/drivers/mouse.c -o $BUILD/mouse.o
$CC $CFLAGS -I$SRC/include -c $SRC/drivers/ata_pio.c -o $BUILD/ata_pio.o
$CC $CFLAGS -I$SRC/include -c $SRC/drivers/heap.c -o $BUILD/heap.o
$CC $CFLAGS -I$SRC/include -c $SRC/drivers/file_system.c -o $BUILD/filesystem.o

$CC $CFLAGS -I$SRC/include -c $SRC/drivers/terminal.c -o $BUILD/terminal.o
$CC $CFLAGS -I$SRC/include -c $SRC/drivers/window.c -o $BUILD/window.o
$CC $CFLAGS -I$SRC/include -c $SRC/drivers/bitmap.c -o $BUILD/bitmap.o


# ---------- LINK ----------
echo "[*] Linking kernel..."
$LD $LDFLAGS \
    $BUILD/boot.o \
    $BUILD/gdt_asm.o \
    $BUILD/idt_asm.o \
    $BUILD/paging.o \
    $BUILD/kstring.o \
    $BUILD/pmm.o \
    $BUILD/vmm.o \
    $BUILD/interrept.o \
    $BUILD/gdt.o \
    $BUILD/idt.o \
    $BUILD/bitmap.o \
    $BUILD/framebuffer.o \
    $BUILD/window.o \
    $BUILD/keyboard.o \
    $BUILD/mouse.o \
    $BUILD/ata_pio.o \
    $BUILD/heap.o \
    $BUILD/terminal.o \
    $BUILD/kernel.o \
    $BUILD/higher_half.o \
    $BUILD/higher_half_jump.o \
    -o $BUILD/myos.bin

# ---------- ISO ----------
echo "[*] Creating ISO..."
mkdir -p $ISO/boot/grub
cp $BUILD/myos.bin $ISO/boot/myos.bin

grub-mkrescue -o $PROJECT_ROOT/$ISO_NAME $ISO

# ---------- RUN ----------
echo "[*] Booting in QEMU..."
qemu-system-i386 \
    -cdrom $PROJECT_ROOT/$ISO_NAME \
    -m 256M \
    -no-reboot \
    -no-shutdown

