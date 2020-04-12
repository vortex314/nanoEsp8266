PROGRAM=nanoEsp8266
DEFINEK ?= -DAAAAA=BBBBBB
EXTRA_COMPONENTS=components/DWM1000 components/Common components/Mqtt components/config extras/sntp extras/mdnsresponder extras/rboot-ota extras/paho_mqtt_c  
PROGRAM_SRC_DIR=. main

PROGRAM_INC_DIR=. main ../nanoAkka/components/wifi ../nanoAkka/main ../esp-open-rtos/include 
PROGRAM_INC_DIR +=../esp-open-rtos/core/include ../esp-open-sdk/lx106-hal/include ../Common  
PROGRAM_INC_DIR +=../ArduinoJson/src ../DWM1000 components/config $(ROOT)bootloader $(ROOT)bootloader/rboot 

PROGRAM_CXXFLAGS += -g -ffunction-sections -fdata-sections -fno-threadsafe-statics 
PROGRAM_CXXFLAGS += -std=c++11 -fno-rtti -lstdc++ -fno-exceptions 
PROGRAM_CXXFLAGS += -DWIFI_PASS=${PSWD} -DWIFI_SSID=${SSID} $(DEFINEK) -DESP_OPEN_RTOS 

PROGRAM_LDFLAGS = -Wl,--gc-sections

ESPBAUD=921600
TTY ?= USB0
SERIAL_PORT ?= /dev/tty$(TTY)
ESPPORT = $(SERIAL_PORT)
SERIAL_BAUD = 115200
# LIBS= m hal gcc stdc++ atomic
LIBS= m  gcc stdc++ 

EXTRA_CFLAGS=-DEXTRAS_MDNS_RESPONDER -DLWIP_MDNS_RESPONDER=1 -DLWIP_NUM_NETIF_CLIENT_DATA=1 -DLWIP_NETIF_EXT_STATUS_CALLBACK=1 
# FLAVOR=sdklike
include ../esp-open-rtos/common.mk

TAG1 :
	touch main/main.cpp
	make DEFINEK=" -DTAG -DHOSTNAME=tag1"

ANCHOR1 :
	touch main/main.cpp
	make DEFINEK=" -DANCHOR=AAA001 -DHOSTNAME=anchor1"

term:
	rm -f $(TTY)_minicom.log
	minicom -D $(SERIAL_PORT) -b $(SERIAL_BAUD) -C $(TTY)_minicom.log

