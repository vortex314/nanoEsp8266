PROGRAM=nanoEsp8266
EXTRA_COMPONENTS=components/Common components/Mqtt extras/sntp extras/mdnsresponder extras/rboot-ota extras/paho_mqtt_c 
PROGRAM_SRC_DIR=. main
PROGRAM_INC_DIR=main ../nanoAkka/main ../esp-open-rtos/include ../esp-open-rtos/core/include ../esp-open-sdk/lx106-hal/include ../Common  ../ArduinoJson/src ../DWM1000 $(ROOT)bootloader $(ROOT)bootloader/rboot ../nanoAkka/components/Wifi
PROGRAM_CXXFLAGS += -fno-threadsafe-statics -std=c++11 -fno-rtti -lstdc++ -fno-exceptions 
PROGRAM_CXXFLAGS += -DWIFI_PASS=${PSWD} -DWIFI_SSID=${SSID} -DESP_OPEN_RTOS
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

term:
	rm -f $(TTY)_minicom.log
	minicom -D $(SERIAL_PORT) -b $(SERIAL_BAUD) -C $(TTY)_minicom.log
