#ifndef _MQTT_WIFI_H_
#define _MQTT_WIFI_H_
#include "lwip/err.h"
#include "lwip/ip_addr.h"
#include "lwip/prot/iana.h"

#include "esp/uart.h"
#include "espressif/esp_common.h"
#include <espressif/esp_sta.h>
#include <espressif/esp_wifi.h>
#include <string.h>

#include <FreeRTOS.h>
#include <task.h>

extern "C" {
#include <paho_mqtt_c/MQTTClient.h>
#include <paho_mqtt_c/MQTTESP8266.h>
}
#include <NanoAkka.h>
#include <Mqtt.h>

#define QOS 0
#define TIMEOUT 10000L



class MqttWifi :  public Mqtt
{
    struct mqtt_network _network;
    mqtt_client_t _client = mqtt_client_default;
    char _mqtt_client_id[20];
    uint8_t _mqtt_buf[512];
    uint8_t _mqtt_readbuf[512];
    mqtt_packet_connect_data_t _data;
    StaticJsonDocument<3000> _jsonBuffer;
    std::string _clientId;
    std::string _address;
    std::string _lwt_topic;
    std::string _lwt_message;
    std::string _hostPrefix;
    TimerSource _reportTimer;
    TimerSource _keepAliveTimer;

    static MqttWifi* _mqttWifi;
    std::string _topicAlive;
    std::string _topicsForDevice;
    TimerSource _yieldTimer;
    static void topic_received_cb(mqtt_message_data_t* md);
    void onMessageReceived(std::string& topic, std::string& payload);


public:
    Sink<bool,2> wifiConnected;
    ValueSource<bool> connected;
    MqttWifi(Thread& thread);
    void init();

    void mqttPublish(const std::string& topic,const  std::string& message);
    void mqttSubscribe(const char* topic);
    bool mqttConnect();
    void mqttDisconnect();

    bool handleMqttMessage(const char* message);

    void onNext(const TimerMsg&);
    void onNext(const MqttMessage&);
    void request();
    template <class T>
    Subscriber<T>& toTopic(const char* name)
    {
        auto flow = new ToMqtt<T>(name);
        *flow >> outgoing;
        return *flow;
    }
    template <class T>
    Source<T>& fromTopic(const char* name)
    {
        auto newSource = new FromMqtt<T>(name);
        incoming >> *newSource;
        return *newSource;
    }
    /*
    			template <class T>
    			MqttFlow<T>& topic(const char* name) {
    				auto newFlow = new MqttFlow<T>(name);
    				incoming >> newFlow->mqttIn;
    				newFlow->mqttOut >> outgoing;
    				return *newFlow;
    			}
    			void observeOn(Thread& thread);*/
};

//_______________________________________________________________________________________________________________
//

#endif
