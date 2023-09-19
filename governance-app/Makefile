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
    ICONNAME=../icons/nanos-concordium-icon.gif
else
    ICONNAME=../icons/nanosplus-concordium-icon.gif
endif

# Main app configuration
APPNAME = CCDGovernance

# Version must be no greater than 99.99.999, otherwise
# extra memory must be allocated in menu.c.
APPVERSION_MAJOR=0
APPVERSION_MINOR=0
APPVERSION_PATCH=1
APPVERSION=$(APPVERSION_MAJOR).$(APPVERSION_MINOR).$(APPVERSION_PATCH)

ifeq ($(TARGET_NAME), TARGET_NANOX)
APP_LOAD_PARAMS=--appFlags 0x200  # APPLICATION_FLAG_BOLOS_SETTINGS
else
APP_LOAD_PARAMS=--appFlags 0x000
endif

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
APP_SOURCE_PATH += src ../src/common
SDK_SOURCE_PATH += lib_stusb lib_stusb_impl

#Import common configuration
include ../Makefile.common
