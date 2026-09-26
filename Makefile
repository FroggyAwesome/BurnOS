# Project & Version Configuration
TARGET = BurnOS
FILEFORMAT = iso
VERSION = 1
PATCHLEVEL = 0
SUBLEVEL = 0
EXTRAVERSION =
FULL_VERSION = $(VERSION).$(PATCHLEVEL).$(SUBLEVEL)$(EXTRAVERSION)

# Toolchain & Architecture Settings
GDBADDR = tcp:localhost:1234
AS = as
CC = gcc
CF = clang-format
ASFLAGS = --32
SU = sudo

# Target Media Configuration & Selection (usb or floppy)
DEVICE_TYPE = usb
DEVICE = sdc

# Paths & File Discovery
INCLUDES = -Iarch -Ibin -Idev -Ietc -Ikernel
CFILES = $(wildcard bin/*.c) $(wildcard dev/*.c) $(wildcard kernel/*.c)
HFILES = $(wildcard bin/*.h) $(wildcard dev/*.h) $(wildcard kernel/*.h)
CONFIGFILES = $(wildcard etc/*.h)

# Compiler Flags & Warnings
NO_BUILTINS = -ffreestanding -fno-pie -fno-stack-protector -fno-builtin -fno-common -fno-unwind-tables -fno-asynchronous-unwind-tables
NO_SIMD = -mno-sse -mno-mmx -mno-sse2
STD = -std=gnu99
COMMON_CFLAGS = $(STD) -pipe -m32 $(NO_BUILTINS) $(NO_SIMD) -fno-strict-aliasing -MMD -MP $(INCLUDES)
CFLAGS = $(COMMON_CFLAGS) -Os -ffunction-sections -fdata-sections -fomit-frame-pointer
CDEBUGFLAGS = $(COMMON_CFLAGS) -O0 -g -fno-omit-frame-pointer
FLAGS ?= $(CDEBUGFLAGS)
CHECK_BASE = $(STD) -m32 $(NO_BUILTINS) $(NO_SIMD) -fsyntax-only $(INCLUDES)
CHECK_WARNINGS = -Wall -Wextra -Wpedantic -Werror -Wshadow -Wundef -Wwrite-strings \
                 -Wcast-align -Wpointer-arith -Wstrict-prototypes -Wredundant-decls -Wnested-externs \
                 -Wcast-qual -Wmissing-prototypes -Wstrict-overflow=2 -Wvla -Winit-self \
                 -Wnull-dereference -Wold-style-definition -Wmissing-declarations
CCHECKFLAGS = $(CHECK_BASE) $(CHECK_WARNINGS) $(CFILES)
CFORMATFLAGS = -i $(CFILES) $(HFILES) $(CONFIGFILES)

# QEMU Emulator Configuration
EMU = qemu-system-i386
KVM = -enable-kvm
EMUCPU = -cpu 486,vendor="SOFT_EMU_486" -smp 1
EMUKVMCPU = -cpu host,vendor="KVM_EMU_HOST",pmu=off -smp 1
EMUMEM = -m 512M
EMUOPTS = -net none -nodefaults -machine pc -boot d
EMUDISPLAY0 = -none
EMUDISPLAY1 = -vga std
EMULOGFILE = build/qemu.log
EMULOG = -d int,cpu_reset -D $(EMULOGFILE)
EMUFLAGS = $(EMUCPU) $(EMUMEM) $(EMUOPTS) $(EMUDISPLAY1)
EMUKVMFLAGS = $(KVM) $(EMUKVMCPU) $(EMUMEM) $(EMUOPTS) $(EMUDISPLAY1)

# Docker Container Configuration
IMAGE_NAME = burnos-builder

# Object Files
OBJS = build/objs/arch/boot.o build/objs/arch/isr.o $(patsubst %.c,build/objs/%.o,$(CFILES))

# PHONY Targets Declaration
.PHONY: all debug build docker format check perms flash iso run run-kvm run-debug version clean-objs clean-bin clean-iso clean-os clean-log clean-version distclean

# Main Build Targets
all: check format distclean version build/bin/kerneldbg.bin build/bin/kernelstd.bin
build/bin/kerneldbg.bin: $(OBJS)
	@mkdir -p build/bin/
	ld -m elf_i386 -T arch/linker.ld -o $@ $^
build/bin/kernelstd.bin: $(OBJS)
	@mkdir -p build/bin/
	ld -m elf_i386 -T arch/linker.ld -o $@ $^
	strip --strip-debug build/bin/kernelstd.bin
debug: FLAGS = $(CDEBUGFLAGS)
debug: version build/bin/kerneldbg.bin
build: FLAGS = $(CFLAGS)
build: check format distclean version build/bin/kernelstd.bin
docker:
	@echo "[+] Building Docker image..."
	docker build -t $(IMAGE_NAME) .
	@echo "[+] Compiling $(TARGET) inside container..."
	docker run --rm --user $$(id -u):$$(id -g) -v "$(PWD):/workspace" $(IMAGE_NAME) make all iso
	@echo "[Done!] ISO image generated in build/out/$(TARGET).iso"

# Compilation Pattern Rules
build/objs/arch/boot.o: arch/boot.s
	@mkdir -p build/objs/arch
	$(AS) $(ASFLAGS) $< -o $@
build/objs/arch/isr.o: arch/isr.s
	@mkdir -p build/objs/arch
	$(AS) $(ASFLAGS) $< -o $@
build/objs/bin/%.o: bin/%.c
	@mkdir -p build/objs/bin
	$(CC) $(FLAGS) -c $< -o $@
build/objs/dev/%.o: dev/%.c
	@mkdir -p build/objs/dev
	$(CC) $(FLAGS) -c $< -o $@
build/objs/kernel/%.o: kernel/%.c
	@mkdir -p build/objs/kernel
	$(CC) $(FLAGS) -c $< -o $@

# Development & Utility Tasks
format:
	$(CF) $(CFORMATFLAGS)
check:
	$(CC) $(CCHECKFLAGS)
perms:
	@echo "[+] Adjusting ownership and permissions..."
	$(SU) chown -R $(USER):$(USER) .
	find . -type d -exec chmod 755 {} +
	find . -type f -exec chmod 644 {} +
	@echo "[Done!] Permissions corrected successfully."
flash:
	@if [ "$(DEVICE_TYPE)" = "usb" ]; then \
		echo "[+] Flashing $(TARGET) to USB (/dev/$(DEVICE))..."; \
		$(SU) dd if=build/out/$(TARGET).$(FILEFORMAT) of=/dev/$(DEVICE) bs=4M status=progress conv=fdatasync; \
	elif [ "$(DEVICE_TYPE)" = "floppy" ]; then \
		echo "[+] Writing $(TARGET) to Floppy (/dev/$(DEVICE))..."; \
		$(SU) dd if=build/out/$(TARGET).$(FILEFORMAT) of=/dev/$(DEVICE) bs=512 status=progress conv=fdatasync; \
	else \
		echo "Error: Unknown DEVICE_TYPE. Choose 'usb' or 'floppy'."; \
		exit 1; \
	fi
	@echo "[Done!] $(TARGET) successfully written to $(DEVICE_TYPE)."

# ISO & Emulation Targets
iso:
	@mkdir -p build/out/ build/iso/boot/grub/
	@if [ -f build/bin/kerneldbg.bin ] && [ -f build/bin/kernelstd.bin ]; then \
		cp build/bin/kerneldbg.bin build/iso/boot/kerneldbg.bin; \
		cp build/bin/kernelstd.bin build/iso/boot/kernelstd.bin; \
	elif [ -f build/bin/kerneldbg.bin ]; then \
		cp build/bin/kerneldbg.bin build/iso/boot/kerneldbg.bin; \
	elif [ -f build/bin/kernelstd.bin ]; then \
		cp build/bin/kernelstd.bin build/iso/boot/kernelstd.bin; \
	else \
		echo "Error: No kernel binary found."; \
		exit 1; \
	fi
	cp boot/grub.cfg build/iso/boot/grub/grub.cfg
	grub-mkrescue -o build/out/$(TARGET).iso build/iso
run:
	$(EMU) $(EMUFLAGS) -cdrom build/out/$(TARGET).$(FILEFORMAT)
run-kvm:
	$(EMU) $(EMUKVMFLAGS) -cdrom build/out/$(TARGET).$(FILEFORMAT)
run-debug:
	$(EMU) $(EMUFLAGS) -gdb $(GDBADDR) -S $(EMULOG) -cdrom build/out/$(TARGET).$(FILEFORMAT)

# Version Management
version:
	@echo "[+] Updating project name to $(TARGET)..."
	@sed -i "s/#define NAME \".*\"/#define NAME \"$(TARGET)\"/" etc/os.h
	@echo "[Done!] Project name updated successfully."
	@echo "[+] Updating project version to v$(FULL_VERSION)..."
	@sed -i "s/#define VERSION \".*\"/#define VERSION \"v$(FULL_VERSION)\"/" etc/os.h
	@echo "[Done!] Project version updated successfully."

# Cleanup Targets
clean-objs:
	@echo "[+] Cleaning object files..."
	@rm -rf build/objs/
	@echo "[Done!] Object files removed successfully."
clean-bin:
	@echo "[+] Cleaning binary files..."
	@rm -rf build/bin/
	@echo "[Done!] Binary files removed successfully."
clean-iso:
	@echo "[+] Cleaning ISO build directories..."
	@rm -rf build/iso/
	@echo "[Done!] ISO directory removed successfully."
clean-os:
	@echo "[+] Cleaning output OS images..."
	@rm -rf build/out/
	@echo "[Done!] Output images removed successfully."
clean-log:
	@echo "[+] Cleaning QEMU logs..."
	@rm -f $(EMULOGFILE)
	@echo "[Done!] Log files removed successfully."
clean-version:
	@echo "[+] Resetting version..."
	@sed -i "s/#define VERSION \".*\"/#define VERSION \"NULL\"/" etc/os.h
	@echo "[Done!] Version reset successfully."
distclean: clean-version
	@echo "[+] Performing deep clean (removing entire build directory)..."
	@rm -rf build/
	@echo "[Done!] Full cleanup completed successfully."

# Automatic Dependencies
-include $(OBJS:.o=.d)
