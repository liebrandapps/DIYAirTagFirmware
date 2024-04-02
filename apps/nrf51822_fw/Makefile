PROJECT_NAME = $(shell basename "$(realpath ./)")

APPLICATION_SRCS = softdevice_handler.c
APPLICATION_SRCS += $(notdir $(wildcard ./*.c))
APPLICATION_SRCS += app_util_platform.c
APPLICATION_SRCS += nrf_drv_common.c
APPLICATION_SRCS += nrf_drv_clock.c
APPLICATION_SRCS += app_error.c
APPLICATION_SRCS += app_error_weak.c
APPLICATION_SRCS += app_timer.c
APPLICATION_SRCS += nrf_drv_gpiote.c

USE_NRF52 := $(filter nrf52,$(MAKECMDGOALS))

SDK_VERSION = 12

SHELL:=/bin/bash


# BOARD_SIMPLE is the default board with no external crystal to maximize compatibility
# BOARD_ALIEXPRESS is for this "AliExpress beacon" https://www.aliexpress.com/item/32826502025.html
# ADV_KEY_BASE64 the advertisment key(base64) from OpenHaystack app

ADV_KEY_BASE64 ?=
BOARD ?= BOARD_SIMPLE
NEW_ADV_KEY_HEX := $(if $(ADV_KEY_BASE64),$(shell base64 -D <<< $(ADV_KEY_BASE64) | xxd -p),)

# Compile for nrf52 by default, use "NRF_MODEL=nrf51 make" to compile for nrf51 platform
ifeq ($(NRF_MODEL), nrf51)
SOFTDEVICE_MODEL = s130
else
SOFTDEVICE_MODEL = s132
NRF_MODEL = nrf52
endif

default: all

build: clean all
	mkdir -p compiled
ifeq ($(NRF_MODEL), nrf51)
	mergehex -m ../../nrf5x-base/sdk/nrf51_sdk_11.0.0/components/softdevice/s130/hex/s130_nrf51_2.0.0_softdevice.hex _build/nrf51822_fw_s130.hex -o compiled/$(NRF_MODEL)_firmware.hex
else
	mergehex -m ../../nrf5x-base/sdk/nrf51_sdk_11.0.0/components/softdevice/s132/hex/s132_nrf52_2.0.0_softdevice.hex _build/openhaystack-alternative_s132.hex -o compiled/$(NRF_MODEL)_firmware.hex
endif
ifneq (, $(shell which objcopy))
	objcopy --input-target=ihex --output-target=binary compiled/$(NRF_MODEL)_firmware.hex compiled/$(NRF_MODEL)_firmware.bin
else ifneq (, $(shell which gobjcopy))
	gobjcopy --input-target=ihex --output-target=binary compiled/$(NRF_MODEL)_firmware.hex compiled/$(NRF_MODEL)_firmware.bin
else ifneq (, $(shell which hex2bin.py))
	hex2bin.py compiled/$(NRF_MODEL)_firmware.hex compiled/$(NRF_MODEL)_firmware.bin
else
	$(error Unable to find (g)objcopy or hex2bin.py. Please install binutils or hex2bin.py to generate a compiled binary.)
endif


patch:
ifneq ($(NEW_ADV_KEY_HEX),)
	xxd -p -c 1000000 < compiled/$(NRF_MODEL)_firmware.bin | \
		sed 's/$(shell echo -n 'OFFLINEFINDINGPUBLICKEYHERE!' | xxd -p)/$(NEW_ADV_KEY_HEX)/' | \
		xxd -r -p > compiled/$(NRF_MODEL)_firmware_patched.bin
endif

LIBRARY_PATHS += .
#SOURCE_PATHS += .

NRF_BASE_PATH ?= ../../nrf5x-base
include $(NRF_BASE_PATH)/make/Makefile
