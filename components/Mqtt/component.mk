#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)
# COMPONENT_ADD_INCLUDEDIRS=.
# CXXFLAGS +="-DESP32_IDF=1"
# Component makefile for extras/Mqtt

# expected anyone using bmp driver includes it as 'Mqtt/MQTT*.h'
INC_DIRS += $(Mqtt_ROOT) ../Common $(Mqtt_ROOT)/../../main $(Mqtt_ROOT)/../../../nanoAkka/main  ../../../ArduinoJson/src

# args for passing into compile rule generation
Mqtt_SRC_DIR =  $(Mqtt_ROOT)
Mqtt_CFLAGS= -DESP_OPEN_RTOS 
Mqtt_CXXFLAGS= -ffunction-sections -fdata-sections  -fno-threadsafe-statics -std=c++11 -fno-rtti -lstdc++ -fno-exceptions 
Mqtt_CXXFLAGS += -DWIFI_PASS=${PSWD} -DWIFI_SSID=${SSID} -DESP_OPEN_RTOS
Mqtt_INC_DIR= ../../main ../ArduinoJson/src ../nanoAkka/components/Wifi

$(eval $(call component_compile_rules,Mqtt))

