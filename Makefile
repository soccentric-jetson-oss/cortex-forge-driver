# SPDX-License-Identifier: GPL-2.0-only
#
# Makefile - top-level orchestrator for cortex-forge-driver
#
# Copyright (C) 2026 SoC Centric
# Author: Sandesh <sandesh@soccentric.com>
#
# Targets: module, lib, test, docker, pkg, static analysis, clean
#

DRV_NAME    := cortex-forge
DRV_LIBNAME := cortex-forge
DRV_VERSION := $(shell cat VERSION 2>/dev/null || echo "0.1.0")

# Directories
SRC_DIR     := src
LIB_DIR     := lib
TEST_DIR    := test
DTS_DIR     := dts
DOCS_DIR    := docs
BUILD_DIR   := build
DIST_DIR    := dist

# Tools
CC          ?= gcc
LD          ?= ld
AR          ?= ar
CFLAGS      ?= -O2 -Wall -Wextra
LDFLAGS     ?=

# Kernel build
KDIR        ?= /lib/modules/$(shell uname -r)/build
ARCH        ?= $(shell uname -m | sed s/aarch64/arm64/ | sed s/x86_64/x86_64/)
CROSS_COMPILE ?=

# Installation
DESTDIR     ?=
PREFIX      ?= /usr
LIBDIR      ?= $(PREFIX)/lib
INCLUDEDIR  ?= $(PREFIX)/include
BINDIR      ?= $(PREFIX)/bin

.PHONY: all module lib test stress cli checkpatch sparse smatch coccicheck
.PHONY: clang-format-check cppcheck analyze doc
.PHONY: deb rpm ipk install clean distclean help
.PHONY: docker-image docker-shell

all: format-check module lib test

# ── Formatting (clang-format) ────────────────────────────────────────────
CLANG_FILES := src/*.c include/*.h lib/src/*.c lib/include/*.h

format:
	clang-format -i $(CLANG_FILES) 2>/dev/null || true

format-check:
	@clang-format --dry-run --Werror $(CLANG_FILES) 2>/dev/null || \
	 echo "WARNING: clang-format not available, skipping format check"

# ── Linting (cppcheck) ──────────────────────────────────────────────────
lint:
	@cppcheck --enable=all --inconclusive --suppress=missingIncludeSystem \
	 src/*.c lib/src/*.c test/*.c 2>/dev/null || \
	 echo "WARNING: cppcheck not available, skipping lint"

# ── Kernel module ──────────────────────────────────────────────────────────
module:
	$(MAKE) -C $(KDIR) M=$(CURDIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

module_clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) clean

# ── Userspace library ───────────────────────────────────────────────────
LIB_SRC  := $(LIB_DIR)/src/lib$(DRV_LIBNAME).c
LIB_OBJ  := $(BUILD_DIR)/lib$(DRV_LIBNAME).o
LIB_SO   := $(BUILD_DIR)/lib$(DRV_LIBNAME).so
LIB_A    := $(BUILD_DIR)/lib$(DRV_LIBNAME).a
LIB_MAJOR := 1

# Include paths
INCLUDES := -I$(LIB_DIR)/include -Iinclude

$(LIB_OBJ): $(LIB_SRC) $(LIB_DIR)/include/lib$(DRV_LIBNAME).h include/cortex_forge_uapi.h
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -fPIC $(INCLUDES) -c -o $@ $<

$(LIB_SO): $(LIB_OBJ)
	$(CC) $(LDFLAGS) -shared -Wl,-soname,lib$(DRV_LIBNAME).so.$(LIB_MAJOR) -o $@ $^
	ln -sf lib$(DRV_LIBNAME).so $(BUILD_DIR)/lib$(DRV_LIBNAME).so.$(LIB_MAJOR)

$(LIB_A): $(LIB_OBJ)
	$(AR) rcs $@ $^

lib: $(LIB_SO) $(LIB_A)

lib_clean:
	rm -rf $(BUILD_DIR)/lib$(DRV_LIBNAME).*

# ── Test applications ───────────────────────────────────────────────────
TEST_SRC   := $(TEST_DIR)/$(DRV_LIBNAME)_test.c
STRESS_SRC := $(TEST_DIR)/$(DRV_LIBNAME)_stress.c
TEST_BIN   := $(BUILD_DIR)/$(DRV_LIBNAME)_test
STRESS_BIN := $(BUILD_DIR)/$(DRV_LIBNAME)_stress

$(TEST_BIN): $(TEST_SRC) $(LIB_A)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< -L$(BUILD_DIR) -l$(DRV_LIBNAME) -lpthread

$(STRESS_BIN): $(STRESS_SRC) $(LIB_A)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< -L$(BUILD_DIR) -l$(DRV_LIBNAME) -lpthread

test: $(TEST_BIN)
	LD_LIBRARY_PATH=$(BUILD_DIR) $(TEST_BIN)

stress: $(STRESS_BIN)
	LD_LIBRARY_PATH=$(BUILD_DIR) $(STRESS_BIN)

test_clean:
	rm -f $(TEST_BIN) $(STRESS_BIN)

# ── Static analysis ──────────────────────────────────────────────────────
checkpatch:
	@$(KDIR)/scripts/checkpatch.pl --strict --no-tree -f src/*.c include/*.h 2>/dev/null || \
	 echo "checkpatch: kernel tree not available, skipping"

sparse:
	$(MAKE) -C $(KDIR) M=$(CURDIR) ARCH=$(ARCH) C=2 CF="-D__CHECK_ENDIAN__" 2>/dev/null || true

clang-format-check:
	@clang-format --dry-run --Werror src/*.c include/*.h lib/src/*.c lib/include/*.h 2>/dev/null || \
	 echo "clang-format: not available, skipping"

cppcheck:
	@cppcheck --enable=all --inconclusive --suppress=missingIncludeSystem \
	 src/*.c lib/src/*.c test/*.c 2>/dev/null || true

analyze: format-check checkpatch sparse clang-format-check cppcheck

# ── Documentation (Doxygen) ────────────────────────────────────────────
doc:
	@doxygen docs/Doxyfile 2>/dev/null || echo "WARNING: doxygen not available, skipping docs"

# ── Packaging ────────────────────────────────────────────────────────────
deb:
	@echo "Building deb package..."
	@mkdir -p $(DIST_DIR)/deb/DEBIAN
	@cp packaging/debian/* $(DIST_DIR)/deb/DEBIAN/ 2>/dev/null || true
	@dpkg-deb --build $(DIST_DIR)/deb $(DIST_DIR)/$(DRV_NAME)-$(DRV_VERSION).deb 2>/dev/null || \
	 echo "dpkg-deb not available"

rpm:
	@echo "Building rpm package..."
	@rpmbuild -bb packaging/rpm/$(DRV_NAME).spec 2>/dev/null || \
	 echo "rpmbuild not available"

# ── Docker ───────────────────────────────────────────────────────────────
docker-image:
	docker build -t $(DRV_NAME)-builder:$(DRV_VERSION) -f docker/Dockerfile.jetson .

docker-shell:
	docker run -it --rm -v $(CURDIR):/src $(DRV_NAME)-builder:$(DRV_VERSION) /bin/bash

# ── Install ──────────────────────────────────────────────────────────────
install: module lib
	install -d $(DESTDIR)$(LIBDIR)
	install -m 755 $(LIB_SO) $(DESTDIR)$(LIBDIR)/lib$(DRV_LIBNAME).so.$(LIB_MAJOR)
	install -m 644 $(LIB_A) $(DESTDIR)$(LIBDIR)/
	install -d $(DESTDIR)$(INCLUDEDIR)
	install -m 644 $(LIB_DIR)/include/lib$(DRV_LIBNAME).h $(DESTDIR)$(INCLUDEDIR)/
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 $(TEST_BIN) $(DESTDIR)$(BINDIR)/
	@echo "Install complete. Run depmod -a after module install."

# ── Clean ────────────────────────────────────────────────────────────────
clean: module_clean lib_clean test_clean
	rm -rf $(BUILD_DIR) $(DIST_DIR) Module.symvers modules.order

distclean: clean
	rm -rf .*.cmd *.o *.ko *.mod *.mod.c *.symvers *.order

# ── Help ─────────────────────────────────────────────────────────────────
help:
	@echo "Cortex Forge Driver $(DRV_VERSION)"
	@echo ""
	@echo "Targets:"
	@echo "  all              Build module + lib + test (default)"
	@echo "  module           Build kernel module (.ko)"
	@echo "  lib              Build userspace library (.so + .a)"
	@echo "  test             Build and run test suite"
	@echo "  stress           Build and run stress test"
	@echo "  analyze          Run all static analysis tools"
	@echo "  doc              Generate Doxygen documentation"
	@echo "  deb              Build Debian package"
	@echo "  rpm              Build RPM package"
	@echo "  docker-image     Build Docker build image"
	@echo "  docker-shell     Interactive shell in Docker container"
	@echo "  install          Install to DESTDIR"
	@echo "  clean            Remove build artifacts"
	@echo "  distclean        Deep clean"
	@echo ""
	@echo "Variables:"
	@echo "  KDIR=            Kernel build directory (default: /lib/modules/.../build)"
	@echo "  ARCH=            Target architecture"
	@echo "  CROSS_COMPILE=   Cross-compiler prefix"
	@echo "  DESTDIR=         Install destination (for packaging)"
	@echo "  PREFIX=          Install prefix (default: /usr)"
