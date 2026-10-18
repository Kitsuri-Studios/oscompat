BUILD_DIR ?= build
BUILD_TYPE ?= Release
LINUX_DIR := tests/linux/build
WIN_DIR := tests/windows/build
ANDROID_DIR := tests/android/wrapdltest
CMAKE ?= cmake

.PHONY: all configure build install clean distclean linux linux-test windows android help

all: build

configure:
	$(CMAKE) -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)

build: configure
	$(CMAKE) --build $(BUILD_DIR) -j

install: build
	$(CMAKE) --install $(BUILD_DIR)

linux:
	$(CMAKE) -B $(LINUX_DIR) -S tests/linux -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	$(CMAKE) --build $(LINUX_DIR) -j

linux-test: linux
	./$(LINUX_DIR)/wrapdl_test

windows:
	$(CMAKE) -B $(WIN_DIR) -S tests/windows -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	$(CMAKE) --build $(WIN_DIR) -j

android:
	cd $(ANDROID_DIR) && ./gradlew.bat assembleDebug

clean:
	-$(CMAKE) --build $(BUILD_DIR) --target clean 2>/dev/null || true
	rm -rf $(BUILD_DIR) cmake-build-debug cmake-build-release

distclean: clean
	rm -rf $(LINUX_DIR) $(WIN_DIR)
	rm -rf $(ANDROID_DIR)/.gradle $(ANDROID_DIR)/build
	rm -rf $(ANDROID_DIR)/app/build $(ANDROID_DIR)/app/.cxx
	find $(ANDROID_DIR) -name ".cxx" -type d -prune -exec rm -rf {} + 2>/dev/null || true

help:
	@echo "targets:"
	@echo "  all          configure + build library (default)"
	@echo "  build        build library in $(BUILD_DIR)"
	@echo "  install      install library + cmake package"
	@echo "  linux        build linux tests"
	@echo "  linux-test   build + run linux tests"
	@echo "  windows      build windows tests"
	@echo "  android      build android test apk"
	@echo "  clean        remove root build dirs"
	@echo "  distclean    clean + all test build dirs"
