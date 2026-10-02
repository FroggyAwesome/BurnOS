# Project & Version Configuration
TARGET = BurnOS
VERSION = 2.00
EXTRAVERSION = -rc1
FULL_VERSION = $(VERSION)$(EXTRAVERSION)

# Toolchain & Architecture Settings
GDBADDR = tcp:localhost:1234
AS = as
CC = gcc
CF = clang-format
ASFLAGS = --32
SU = sudo

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
EMUMSG = @echo "[+] Launching QEMU..."
EMU = qemu-system-i386
EMUCPU = -cpu 486,vendor="SOFT_EMU_486" -smp 1
EMUMEM = -m 512M
EMUOPTS = -net none -nodefaults -machine pc
EMUDISPLAY0 = -none
EMUDISPLAY1 = -vga std
EMULOGFILE = build/qemu.log
EMULOG = -d int,cpu_reset -D $(EMULOGFILE)
EMUFLAGS = $(EMUCPU) $(EMUMEM) $(EMUOPTS) $(EMUDISPLAY1)

# Bootloader / GRUB Configuration
CFG_SRC ?= boot/default-grub/grub.cfg

# Docker Container Configuration
IMAGE_NAME = burnos-builder

# Object Files
OBJS = build/objs/arch/boot.o build/objs/arch/isr.o $(patsubst %.c,build/objs/%.o,$(CFILES))

# PHONY Targets Declaration
.PHONY: all debug build docker format check perms img iso iso-default-grub iso-fast-grub run run-iso run-img run-debug run-debug-iso run-debug-img version clean-objs clean-bin clean-iso clean-os clean-log clean-version distclean

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
	docker run --rm --user $$(id -u):$$(id -g) -v "$(PWD):/workspace" $(IMAGE_NAME) make all iso img

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
	@$(SU) chown -R $(USER):$(USER) .
	@find . -type d -exec chmod 755 {} +
	@find . -type f -exec chmod 644 {} +
	@echo "[Done!] Permissions corrected successfully."
img:
	@echo "[+] Writing $(TARGET).iso to img file..."
	@dd if=build/out/$(TARGET).iso of=build/out/$(TARGET).img bs=4M status=progress conv=fdatasync
	@echo "[Done!] $(TARGET).iso successfully written to $(TARGET).img."

# ISO & Emulation Targets
iso:
	@$(MAKE) --no-print-directory CFG_SRC=boot/default-grub/grub.cfg iso-default-grub
iso-default-grub:
	@echo "[+] Building ISO image..."
	@mkdir -p build/out/ build/iso/boot/grub/
	@if [ -f build/bin/kerneldbg.bin ] && [ -f build/bin/kernelstd.bin ]; then \
		echo "[+] Copying both kernels (dbg and std)"; \
		cp build/bin/kerneldbg.bin build/iso/boot/kerneldbg.bin; \
		cp build/bin/kernelstd.bin build/iso/boot/kernelstd.bin; \
	elif [ -f build/bin/kerneldbg.bin ]; then \
		echo "[+] Copying dbg kernel"; \
		cp build/bin/kerneldbg.bin build/iso/boot/kerneldbg.bin; \
	elif [ -f build/bin/kernelstd.bin ]; then \
		echo "[+] Copying std kernel"; \
		cp build/bin/kernelstd.bin build/iso/boot/kernelstd.bin; \
	else \
		echo "[Error] No kernel binary found."; \
		exit 1; \
	fi
	@echo "[+] Copying grub.cfg from $(CFG_SRC)..."
	@cp $(CFG_SRC) build/iso/boot/grub/grub.cfg
	@echo "[+] Generating ISO image with grub-mkrescue..."
	grub-mkrescue -o build/out/$(TARGET).iso build/iso
	@echo "[Done!] ISO image successfully built"
iso-fast-grub:
	@$(MAKE) --no-print-directory CFG_SRC=boot/fast-grub/grub.cfg iso-fast-grub
run: run-iso
run-iso:
	$(EMUMSG)
	@$(EMU) $(EMUFLAGS) -boot d -cdrom build/out/$(TARGET).iso
run-img:
	$(EMUMSG)
	@$(EMU) $(EMUFLAGS) -hda build/out/$(TARGET).img
run-debug: run-debug-iso
run-debug-iso:
	$(EMUMSG)
	@$(EMU) $(EMUFLAGS) -gdb $(GDBADDR) -S $(EMULOG) -boot d -cdrom build/out/$(TARGET).iso
run-debug-img:
	$(EMUMSG)
	@$(EMU) $(EMUFLAGS) -gdb $(GDBADDR) -S $(EMULOG) -hda build/out/$(TARGET).img

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
