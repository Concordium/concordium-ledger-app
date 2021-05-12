#*******************************************************************************
#   Ledger Blue
#   (c) 2016 Ledger
#
#	Modifications (c) 2021 Concordium
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

export BOLOS_SDK = nanos-secure-sdk
include $(BOLOS_SDK)/Makefile.defines

# Main app configuration
APPNAME = "Concordium"
ICONNAME = nanos-concordium-icon.gif
APPVERSION = 0.2.0

APP_LOAD_PARAMS = --appFlags 0x00 $(COMMON_LOAD_PARAMS)

# Restrict derivation paths to the Concordium specific path.
APP_LOAD_PARAMS += --path "1105'/0'"

# Restrict derivation to only be able to use ed25519
APP_LOAD_PARAMS +=--curve ed25519

# Build configuration
APP_SOURCE_PATH += src
SDK_SOURCE_PATH += lib_stusb lib_stusb_impl

DEFINES += APPVERSION=\"$(APPVERSION)\"

DEFINES += OS_IO_SEPROXYHAL IO_SEPROXYHAL_BUFFER_SIZE_B=128
DEFINES += HAVE_BAGL HAVE_SPRINTF
DEFINES += PRINTF\(...\)=

DEFINES += HAVE_IO_USB HAVE_L4_USBLIB IO_USB_MAX_ENDPOINTS=7 IO_HID_EP_LENGTH=64 HAVE_USB_APDU

# Both nano S and X benefit from the flow.
DEFINES += HAVE_UX_FLOW

# Use stack canary for development. Will reboot device if a stack overflow is detected.
# DEFINES += HAVE_BOLOS_APP_STACK_CANARY

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
load: APP_LOAD_PARAMS +=--signApp --signPrivateKey $(TEST_LEDGER_SIGNING_KEY) --rootPrivateKey $(TEST_LEDGER_SIGNING_KEY)
endif

# Pre-requisities for building a release
ifndef LEDGER_SIGNING_KEY
release: $(error The release signing key must be set when building a release.)
endif

# The public-key should be taken from an env variable, and should match the used signing key.
export PUBLIC_KEY=047ea50b13442984a4cb9bab86016812aa023c25f542a1e727ec5908a65c78d6582d19efe88e96ae4150dea09b0c8b5767524db4afc98ee2bafeed1cc2f8382743

# Main rules
all: default

release: all
	@echo
	@echo "CONCORDIUM LEDGER APP RELEASE BUILD"
	@echo
	@echo $(APPNAME)
	@echo "Version $(APPVERSION)"
	@echo "Target firmware version $(TARGET_VERSION)"
	python3 -m ledgerblue.loadApp $(APP_LOAD_PARAMS) --offline signed_app.apdu --signApp --signPrivateKey $(LEDGER_SIGNING_KEY)
	@echo 
	@echo "Signing and packaging application for release"
	@echo "python3 -m ledgerblue.loadApp $(APP_LOAD_PARAMS) --signature `cat signed_app.apdu | tail -1 | cut -c15-`" >> install.bat
	@echo "python3 -m ledgerblue.setupCustomCA --targetId $(TARGET_ID) --public $(PUBLIC_KEY) --name concordium" >> loadcertificate.bat
	@echo "#!/bin/bash" >> install.sh
	@echo "python3 -m ledgerblue.loadApp $(APP_LOAD_PARAMS) --signature `cat signed_app.apdu | tail -1 | cut -c15-`" >> install.sh
	@chmod +x install.sh
	@echo "#!/bin/bash" >> loadcertificate.sh
	@echo "python3 -m ledgerblue.setupCustomCA --targetId $(TARGET_ID) --public $(PUBLIC_KEY) --name concordium" >> loadcertificate.sh
	@chmod +x loadcertificate.sh
	@echo "#!/bin/bash" >> uninstall.sh
	@echo "python3 -m ledgerblue.deleteApp $(COMMON_DELETE_PARAMS)" >> uninstall.sh
	@chmod +x uninstall.sh
	@chmod +x bin/app.hex
	@zip concordium-ledger-app-$(APPVERSION)-target-$(TARGET_VERSION).zip \
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
	@echo "Application was successfully signed and packaged to concordium-ledger-app-$(APPVERSION)-target-$(TARGET_VERSION).zip"

load: all
	python3 -m ledgerblue.loadApp $(APP_LOAD_PARAMS)

delete:
	python3 -m ledgerblue.deleteApp $(COMMON_DELETE_PARAMS)

# Import rules to compile the glyphs supplied in the glyphs/ directory
include $(BOLOS_SDK)/Makefile.glyphs

# Import generic rules from the SDK
include $(BOLOS_SDK)/Makefile.rules
