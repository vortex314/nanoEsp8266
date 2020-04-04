#include "MqttWifi.h"


//#define MQTT_PORT 1883
// .match(Exit, [](Msg& msg) { exit(0); })
#define MQTT_HOST "limero.ddns.net"
#define MQTT_PORT 1883
#define MQTT_USER ""

#define MQTT_PASS ""

enum {
    WIFI_CONNECTED = 0,    // 0
    WIFI_DISCONNECTED,     // 1
    MQTT_CONNECTED,        // 2
    SIG_MQTT_DISCONNECTED, // 3
    MQTT_SUBSCRIBED,       // 4
    MQTT_PUBLISHED,        // 5
    MQTT_INCOMING,         // 6
    SIG_MQTT_FAILURE,      // 7
    MQTT_DO_PUBLISH        // 8
};

MqttWifi* MqttWifi::_mqttWifi = 0;


MqttWifi::MqttWifi(Thread& thr) : Mqtt(thr),_yieldTimer(thr,1,5000,true)
{
//	_address = va_arg(args, const char*);
    _address = MQTT_HOST;
    wifiConnected.on(false);
    connected = false;
    _mqttWifi = this;
}

void MqttWifi::init()
{
    string_format(_hostPrefix, "src/%s/", Sys::hostname());

    _topicAlive = "src/";
    _topicAlive += Sys::hostname();
    _topicAlive += "/system/alive";

    _topicsForDevice = "dst/";
    _topicsForDevice += Sys::hostname();
    _topicsForDevice += "/#";

    connected = false;
    wifiConnected.async(thread(),[&](bool conn) {
        if(conn) {
            if ( !connected()) mqttConnect();
        } else {
            if ( connected() )	mqttDisconnect();
        }
    });
    outgoing.async(thread(),[&](const MqttMessage& m) {
        std::string topic = _hostPrefix;
        topic += m.topic;
        mqttPublish(topic,m.message);
    });

    _yieldTimer >> ([&](const TimerMsg& tm) {
        if(connected()) {
            int ret = mqtt_yield(&_client, 0);
            if(ret == MQTT_DISCONNECTED) {
                mqttDisconnect();
                mqttConnect();
            }
        } else if(wifiConnected()) {
            mqttConnect();
        }
    });

}

void MqttWifi::topic_received_cb(mqtt_message_data_t* md)
{
    static bool busy = false;
    if (!busy) {
        mqtt_message_t* message = md->message;
        std::string topic((char*) md->topic->lenstring.data, (uint32_t) md
                          ->topic->lenstring.len);
        std::string data((char*) (message->payload), (uint32_t) message
                         ->payloadlen);
        busy = true;
        _mqttWifi->incoming.on({topic,data});
        busy = false;
    } else {
        WARN(" sorry ! MQTT reception busy ");
    }
}

void MqttWifi::mqttPublish(const std::string& topic,const std::string& msg)
{
    if (connected()) {
        uint64_t start=Sys::millis();
//		INFO(" MQTT TXD : %s = %s", topic.c_str(), msg.c_str());
        mqtt_message_t message;
        message.payload = (void*) msg.data();
        message.payloadlen = msg.size();
        message.dup = 0;
        message.qos = MQTT_QOS0;
        message.retained = 0;
        int _ret = mqtt_publish(&_client, topic.c_str(), &message);
        if (_ret != MQTT_SUCCESS) {
            INFO("error while publishing message: %d", _ret);
            mqttDisconnect();
            return;
        }
        _ret = mqtt_yield(&_client,0);
        if (_ret == MQTT_DISCONNECTED ) {
            WARN(" mqtt_yield failed : %d ", _ret);
            mqttDisconnect();
        }
//		INFO(" delay MQTT publish %s %d ",topic.c_str(),(uint32_t)(Sys::millis()-start));
    } else {
//		WARN(" cannot publish : disconnected. ");
    }
}

void MqttWifi::mqttSubscribe(const char* topic)
{
    INFO("mqqt subscribe topic %s ", topic);
    mqtt_subscribe(&_client, topic, MQTT_QOS1, topic_received_cb);
}

bool MqttWifi::mqttConnect()
{
    connected = false;
    int _ret = 0;
    _client = mqtt_client_default;
    _data = mqtt_packet_connect_data_initializer;
    mqtt_network_new(&_network);
    ZERO(_mqtt_client_id);
    strcpy(_mqtt_client_id, Sys::hostname());

    INFO("connecting to MQTT server %s:%d ... ", MQTT_HOST, MQTT_PORT);
    _ret = mqtt_network_connect(&_network, MQTT_HOST, MQTT_PORT);
    if (_ret) {
        INFO("mqtt connect error: %d", _ret);
        return false;
    }
    mqtt_client_new(&_client, &_network, 3000, _mqtt_buf, 1024, _mqtt_readbuf, 1024);

    _data.willFlag = 0;
    _data.MQTTVersion = 3;
    _data.clientID.cstring = _mqtt_client_id;
    _data.username.cstring = (char*) MQTT_USER;
    //   _data.username.lenstring=0;
    _data.password.cstring = (char*) MQTT_PASS;
    //    _data.password.lenstring=0;
    _data.keepAliveInterval = 20;
    _data.cleansession = 0;
    _data.will.topicName.cstring = (char*) _topicAlive.c_str();
    _data.will.message.cstring = (char*) "false";
    _ret = mqtt_connect(&_client, &_data);
    if (_ret) {
        WARN("mqtt_connect() failed.");
        mqtt_network_disconnect(&_network);
        return false;
    };
    INFO("mqtt connected.");
    mqttSubscribe(_topicsForDevice.c_str());
    connected = true;
    return true;
}

void MqttWifi::mqttDisconnect()
{
    INFO(" mqtt disconnecting.");
    mqtt_disconnect(&_client);
    mqtt_network_disconnect(&_network);
    connected = false;
}
