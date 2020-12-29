PROJECT_NAME := limeroEsp8266
DEFINEK ?= -DAAAAA=BBBBBB
IDF_PATH =$(HOME)/esp/ESP8266_RTOS_SDK
LIMERO = $(HOME)/workspace/limero
EXTRA_COMPONENT_DIRS = $(LIMERO)/esp8266_rtos_sdk $(LIMERO)/limero DWM1000 $(LIMERO)/common

CXXFLAGS += -g -ffunction-sections -fdata-sections -fno-threadsafe-statics 
CXXFLAGS += -std=c++11 -fno-rtti -lstdc++ -fno-exceptions 
CXXFLAGS += -DWIFI_PASS=${PSWD} -DWIFI_SSID=${SSID} $(DEFINEK) -DESP8266_RTOS_SDK
CXXFLAGS += -DMQTT_HOST="limero.ddns.net" -DMQTT_PORT=1883
CXXFLAGS += -I $(LIMERO)/inc -I$(HOME)/workspace/ArduinoJson/src


ESPBAUD=921600
TTY ?= USB0
SERIAL_PORT ?= /dev/tty$(TTY)
ESPPORT = $(SERIAL_PORT)
SERIAL_BAUD = 115200

include $(IDF_PATH)/make/project.mk

TAG1 :
	touch main/main.cpp
	make DEFINEK=" -DTAG -DHOSTNAME=tag1"

ANCHOR1 :
	touch main/main.cpp
	make DEFINEK=" -DANCHOR=AAA001 -DHOSTNAME=anchor1"
ANCHOR2 :
	touch main/main.cpp
	make DEFINEK=" -DANCHOR=AAA002 -DHOSTNAME=anchor2"
ANCHOR3 :
	touch main/main.cpp
	make DEFINEK=" -DANCHOR=AAA003 -DHOSTNAME=anchor3"
ANCHOR4 :
	touch main/main.cpp
	make DEFINEK=" -DANCHOR=AAA004 -DHOSTNAME=anchor4"
ANCHOR5 :
	touch main/main.cpp
	make DEFINEK=" -DANCHOR=AAA005 -DHOSTNAME=anchor5"

TREESHAKER:
	touch main/main.cpp
	make DEFINEK=" -DTREESHAKER -DHOSTNAME=treeshaker"

term:
	rm -f $(TTY)_minicom.log
	minicom -D $(SERIAL_PORT) -b $(SERIAL_BAUD) -C $(TTY)_minicom.log

