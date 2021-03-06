#include "ConfigFlow.h"



#ifdef ESP_OPEN_RTOS
extern "C" {
#include <sysparam.h>
#include <espressif/spi_flash.h>
#include "espressif/esp_common.h"
}

bool ConfigStore::_loaded=false;
StaticJsonDocument<1024> ConfigStore::jsonDoc;


bool ConfigStore::loadAll()
{
    if (_loaded) {
        return true;
    }
//    sysparam_set_data("config",(uint8_t*)"{}",3,false);

    sysparam_status_t status;
    uint32_t base_addr, num_sectors;

    status = sysparam_get_info(&base_addr, &num_sectors);
    if (status == SYSPARAM_OK) {
        INFO("[current sysparam region is at 0x%08x (%d sectors)]", base_addr, num_sectors);
    } else {
        num_sectors = DEFAULT_SYSPARAM_SECTORS;
        base_addr = sdk_flashchip.chip_size - (5 + num_sectors) * sdk_flashchip.sector_size;
    }
    uint8_t* destPtr;
    size_t actual_length;
    bool is_binary;
    status = sysparam_get_data("config", &destPtr, &actual_length, &is_binary);
    INFO("sysparam_get_data %s:%d",destPtr,actual_length);
    char* _charBuffer=(char*)(malloc(1000));
    if (status == SYSPARAM_OK) {
        strncpy(_charBuffer,(char*)destPtr,actual_length);
        free(destPtr);
    } else {
        ERROR("sysparam_get_data('config',...) fails : %d ",status);
        sysparam_set_data("config",(uint8_t*)"{}",3,false);
        free(_charBuffer);
        return true;
    }
// load string from flash

    // load JsonObject from String

    _loaded = true;
    auto error = deserializeJson(jsonDoc,_charBuffer);
    if(error) {
        ERROR(" couldn't parse config '%s' , dropped old config ! ",_charBuffer);
        strcpy(_charBuffer,"{}");
        deserializeJson(jsonDoc,"{}");
        saveAll();
    }
    std::string buffer;
    serializeJson(jsonDoc, buffer);
    INFO(" config loaded : %s",buffer.c_str());
    free(_charBuffer);
    return true;
}

bool ConfigStore::saveAll()
{
    std::string buffer;
    serializeJson(jsonDoc,buffer);
    sysparam_status_t status = sysparam_set_string("config", buffer.c_str());
    if ( status == SYSPARAM_OK) {
        INFO(" config saved : %s ", buffer.c_str());
        return true;
    } else {
        ERROR("config save failed : %d ",status);
        return false;
    }
}
#endif
