FROM archlinux:latest

RUN pacman -Syu --noconfirm \
			make \
			gcc \
			clang \
			grub \
			xorriso \
			mtools

WORKDIR /workspace
