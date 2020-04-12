#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)
# COMPONENT_ADD_INCLUDEDIRS=.
# CXXFLAGS +="-DESP32_IDF=1"
# Component makefile for extras/config

# expected anyone using bmp driver includes it as 'config/MQTT*.h'
INC_DIRS +=  ../Common   

# args for passing into compile rule generation
config_SRC_DIR =  $(config_ROOT)
config_CFLAGS= -DESP_OPEN_RTOS 
config_CXXFLAGS= -ffunction-sections -fdata-sections  -fno-threadsafe-statics -std=c++11 
config_CXXFLAGS+=-fno-rtti -lstdc++ -fno-exceptions -DPSWD=${PSWD} -DSSID=${SSID} -DESP_OPEN_RTOS
config_INC_DIR= ../etl/src ../../main ../nanoAkka/components/wifi ../ArduinoJson/src


$(eval $(call component_compile_rules,config))

