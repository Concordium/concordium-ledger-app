#*******************************************************************************
#   Ledger Blue
#   (c) 2016 Ledger
#
#	Modifications (c) 2021 Concordium Software ApS
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#*******************************************************************************

ifndef BOLOS_SDK
$(error Environment variable BOLOS_SDK is not set.)
endif

include $(BOLOS_SDK)/Makefile.defines

ifeq ($(TARGET_NAME),TARGET_NANOS)
    ICONNAME=icons/nanos-concordium-icon.gif
else
    ICONNAME=icons/nanosplus-concordium-icon.gif
endif

# Main app configuration
APPNAME = "Concordium"

# Version must be no greater than 99.99.999, otherwise
# extra memory must be allocated in menu.c.
APPVERSION_MAJOR=4
APPVERSION_MINOR=1
APPVERSION_PATCH=2
APPVERSION=$(APPVERSION_MAJOR).$(APPVERSION_MINOR).$(APPVERSION_PATCH)

ifeq ($(TARGET_NAME), TARGET_NANOX)
APP_LOAD_PARAMS=--appFlags 0x200  # APPLICATION_FLAG_BOLOS_SETTINGS
else
APP_LOAD_PARAMS=--appFlags 0x000
endif

# Ledger: add the "Pending security review" disclaimer
APP_LOAD_PARAMS += --tlvraw 9F:01
DEFINES += HAVE_PENDING_REVIEW_SCREEN

APP_LOAD_PARAMS += $(COMMON_LOAD_PARAMS)

# Restrict derivation paths to the Concordium specific path.
APP_LOAD_PARAMS += --path "1105'/0'"
# Restrict derivation to only be able to use ed25519
APP_LOAD_PARAMS +=--curve ed25519

# Parameters required by the Ledger workflows. Can allow for different
# variants to be built. For the Concordium app we do not use this, and only
# have a single variant.
VARIANT_PARAM = concordium
VARIANT_VALUES = concordium

# Build configuration
APP_SOURCE_PATH += src
SDK_SOURCE_PATH += lib_stusb lib_stusb_impl

ifeq ($(TARGET_NAME),TARGET_NANOS)
	DEFINES += IO_SEPROXYHAL_BUFFER_SIZE_B=128
else
	SDK_SOURCE_PATH += lib_ux
	DEFINES += IO_SEPROXYHAL_BUFFER_SIZE_B=300
	DEFINES += HAVE_GLO096
	DEFINES += HAVE_BAGL BAGL_WIDTH=128 BAGL_HEIGHT=64
	DEFINES += HAVE_BAGL_ELLIPSIS # long label truncation feature
	DEFINES += HAVE_BAGL_FONT_OPEN_SANS_REGULAR_11PX
	DEFINES += HAVE_BAGL_FONT_OPEN_SANS_EXTRABOLD_11PX
	DEFINES += HAVE_BAGL_FONT_OPEN_SANS_LIGHT_16PX
endif

DEFINES += OS_IO_SEPROXYHAL
DEFINES += HAVE_BAGL HAVE_SPRINTF
DEFINES += PRINTF\(...\)=

DEFINES += HAVE_IO_USB HAVE_L4_USBLIB IO_USB_MAX_ENDPOINTS=7 IO_HID_EP_LENGTH=64 HAVE_USB_APDU

# Both nano S and X benefit from the flow.
DEFINES += HAVE_UX_FLOW

DEFINES += APPVERSION=\"$(APPVERSION)\"
# Make the version parameters accessible from the app.
DEFINES += APPVERSION_MAJOR=$(APPVERSION_MAJOR)
DEFINES += APPVERSION_MINOR=$(APPVERSION_MINOR)
DEFINES += APPVERSION_PATCH=$(APPVERSION_PATCH)

# Stop execution after a stack overflow
DEFINES += HAVE_BOLOS_APP_STACK_CANARY

# Bluetooth settings for Nano X
ifeq ($(TARGET_NAME), TARGET_NANOX)
	DEFINES += HAVE_BLE BLE_COMMAND_TIMEOUT_MS=2000 HAVE_BLE_APDU
	SDK_SOURCE_PATH  += lib_blewbxx lib_blewbxx_impl
endif

# Compiler, assembler, and linker
ifneq ($(BOLOS_ENV),)
$(info BOLOS_ENV=$(BOLOS_ENV))
CLANGPATH := $(BOLOS_ENV)/clang-arm-fropi/bin/
GCCPATH := $(BOLOS_ENV)/gcc-arm-none-eabi-5_3-2016q1/bin/
else
$(info BOLOS_ENV is not set: falling back to CLANGPATH and GCCPATH)
endif
ifeq ($(CLANGPATH),)
$(info CLANGPATH is not set: clang will be used from PATH)
endif
ifeq ($(GCCPATH),)
$(info GCCPATH is not set: arm-none-eabi-* will be used from PATH)
endif

CC := $(CLANGPATH)clang
CFLAGS += -O3 -Os

AS := $(GCCPATH)arm-none-eabi-gcc
AFLAGS +=

LD := $(GCCPATH)arm-none-eabi-gcc
LDFLAGS += -O3 -Os
LDLIBS += -lm -lgcc -lc

# If a test signing key has been set, then use that to sign when loading
# the application for development. Otherwise load it without signing.
ifdef TEST_LEDGER_SIGNING_KEY
APP_LOAD_PARAMS +=--signApp --signPrivateKey $(TEST_LEDGER_SIGNING_KEY) --rootPrivateKey $(TEST_LEDGER_SIGNING_KEY)
endif

# The load parameters must be evaluated before being put in the release installation
# scripts, otherwise the application will fail loading (in particular the --path parameter)
# makes things fail as it has to be enclosed with "".
APP_LOAD_PARAMS_EVAL=$(shell printf '\\"%s\\" ' $(APP_LOAD_PARAMS))

# Extract the target device (NANOS/NANOX)
TARGET_DEVICE = $(subst TARGET_,,$(TARGET_NAME))

# Set release filename depending on device being built for
ifeq ($(TARGET_NAME),TARGET_NANOS)
	TARGET_DEVICE=nanos
else
	TARGET_DEVICE=nanos-plus
endif

# Main rules
all: default

release: all
# Fail if trying to build a nano x release, as sideloading is not supported.
ifeq ($(BOLOS_SDK),nanox-secure-sdk)
release: fail_nanox_release
endif
ifeq ($(BOLOS_SDK),nanox-secure-sdk/)
release: fail_nanox_release
endif
# Require a public and private key pair when creating a release, and 
# fail if they are not available
ifndef LEDGER_SIGNING_KEY
release: fail_release_no_signing_key
endif
ifndef LEDGER_PUBLIC_KEY
release: fail_release_no_public_key
endif
	@echo 
	@echo "CONCORDIUM LEDGER APP RELEASE BUILD"
	@echo
	@echo $(APPNAME)
	@echo "Version $(APPVERSION)"
	@echo "Target device $(TARGET_DEVICE)"
	@echo "Target firmware version $(TARGET_VERSION)"
	python3 -m ledgerblue.loadApp $(APP_LOAD_PARAMS) --offline signed_app.apdu --signApp --signPrivateKey $(LEDGER_SIGNING_KEY)
	@echo 
	@echo "Signing and packaging application for release"
	@echo "python3 -m ledgerblue.loadApp $(APP_LOAD_PARAMS_EVAL) --signature `cat signed_app.apdu | tail -1 | cut -c15-`" >> install.bat
	@echo "python3 -m ledgerblue.setupCustomCA --targetId $(TARGET_ID) --public $(LEDGER_PUBLIC_KEY) --name concordium" >> loadcertificate.bat
	@echo "#!/bin/bash" >> install.sh
	@echo "python3 -m ledgerblue.loadApp $(APP_LOAD_PARAMS_EVAL) --signature `cat signed_app.apdu | tail -1 | cut -c15-`" >> install.sh
	@chmod +x install.sh
	@echo "#!/bin/bash" >> loadcertificate.sh
	@echo "python3 -m ledgerblue.setupCustomCA --targetId $(TARGET_ID) --public $(LEDGER_PUBLIC_KEY) --name concordium" >> loadcertificate.sh
	@chmod +x loadcertificate.sh
	@echo "#!/bin/bash" >> uninstall.sh
	@echo "python3 -m ledgerblue.deleteApp $(COMMON_DELETE_PARAMS)" >> uninstall.sh
	@chmod +x uninstall.sh
	@chmod +x bin/app.hex
	@zip -r concordium-ledger-app-$(APPVERSION)-$(TARGET_DEVICE)-$(TARGET_VERSION).zip \
		licenses \
		install.bat \
		loadcertificate.bat \
		install.sh \
		loadcertificate.sh \
		uninstall.sh \
		bin/app.hex
	@rm -f install.bat
	@rm -f loadcertificate.bat
	@rm -f install.sh
	@rm -f loadcertificate.sh
	@rm -f uninstall.sh
	@rm -f signed_app.apdu
	@echo
	@echo "Application was successfully signed and packaged to concordium-ledger-app-$(APPVERSION)-$(TARGET_DEVICE)-$(TARGET_VERSION).zip"

fail_release_no_public_key:
	$(error A public key must set as LEDGER_PUBLIC_KEY)

fail_release_no_signing_key:
	$(error A signing key must set as LEDGER_SIGNING_KEY)

fail_nanox_release:
	$(error The release can only be built for Nano S, as Nano X does not support sideloading.)

emulator: all
	@echo
	@echo "CONCORDIUM LEDGER APP EMULATOR TESTING BUILD"
	@echo $(APPNAME)
	@echo "Version $(APPVERSION)"
	@echo "Target device $(TARGET_DEVICE)"
	@echo "Target firmware version $(TARGET_VERSION)"
	@echo
	@echo "The binary used by the emulator is available at bin/app.elf"

load: all
	python3 -m ledgerblue.loadApp $(APP_LOAD_PARAMS)

delete:
	python3 -m ledgerblue.deleteApp $(COMMON_DELETE_PARAMS)

lint:
	find . -regex './src/.*\.\(c\|h\)\|./unit_tests/.*\.\(c\|h\)' -exec clang-format -style=file -i {} \;

# TODO: Use Makefile.standard_app to get this automatically in the future.
listvariants:
	@echo VARIANTS $(VARIANT_PARAM) $(VARIANT_VALUES)

# Import rules to compile the glyphs supplied in the glyphs/ directory
include $(BOLOS_SDK)/Makefile.glyphs

# Import generic rules from the SDK
include $(BOLOS_SDK)/Makefile.rules
