#include <cstring>
#include <stdio.h>
#include <FreeRTOS.h>
#include <esp/uart.h>
#include <espressif/esp_system.h>
#include <task.h>
#include <NanoAkka.h>


#define GENERIC
//#define DWM1000

/******************************************************************************
 * FunctionName : app_main
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
 *******************************************************************************/

Log logger(256);


void vAssertCalled( unsigned long ulLine, const char * const pcFileName ) {
	printf("Assert called on : %s:%lu",pcFileName,ulLine);
}

extern "C" void vApplicationMallocFailedHook() {
	WARN(" malloc failed ! ");
	while(1);
}

//______________________________________________________________________
//

class Poller : public Actor,public Sink<TimerMsg,2> {
		TimerSource _pollInterval;
		std::vector<Requestable*> _publishers;
		uint32_t _idx=0;
		bool _connected;
	public:
		Sink<bool,2> connected;
		Poller(Thread& t) : Actor(t),_pollInterval(t,1,1000,true) {
			_pollInterval >> this;
			connected.async(thread(),[&](const bool& b) {
				_connected=b;
			});
			async(thread(),[&](const TimerMsg tm) {
				if( _publishers.size() && _connected ) _publishers[_idx++ % _publishers.size()]->request();
			});
		};
		void setInterval(uint32_t t) {
			_pollInterval.interval(t);
		}
		Poller& operator()(Requestable& rq ) {
			_publishers.push_back(&rq);
			return *this;
		}
};
#include <LedBlinker.h>
#include <Wifi.h>
#include <MqttWifi.h>
//------------------------------------------------------------------ actors and threads
Thread mainThread("main");
Thread mqttThread("mqtt");
Poller poller(mainThread);
LedBlinker ledBlue(mainThread,2,UINT32_MAX);
LedBlinker ledRed(mainThread,16,1000);
Wifi wifi(mqttThread);
MqttWifi mqtt(mqttThread);
//------------------------------------------------------------------ system props
ValueSource<std::string> systemBuild("NOT SET");
ValueSource<std::string> systemHostname("NOT SET");
ValueSource<bool> systemAlive=true;
LambdaSource<uint32_t> systemHeap([]() {
	return xPortGetFreeHeapSize();
});
LambdaSource<uint64_t> systemUptime([]() {
	return Sys::millis();
});

#include <ConfigFlow.h>
ConfigFlow<std::string> configHost("system/host","unknown");
ConfigFlow<float> configFloat("system/float",3.141592653);

//-----------------------------------------------------------------------
#include <DWM1000_Tag.h>
#include <DWM1000_Anchor.h>
extern "C" void user_init(void) {
	uart_set_baud(0, 115200);
	sdk_system_update_cpu_freq(SYS_CPU_160MHZ); // need for speed, DWM1000 doesn't wait !
	Sys::init();
	std::string conf;
#ifdef HOSTNAME
	Sys::hostname(S(HOSTNAME));
#else
	std::string hn;
	union {
		uint8_t macBytes[6];
		uint64_t macInt;
	};
	macInt=0L;
	if ( sdk_wifi_get_macaddr(STATION_IF,macBytes) != true) WARN(" esp_base_mac_addr_get() failed.");;
	string_format(hn, "ESP82-%d", macInt & 0xFFFF);
	Sys::hostname(hn.c_str());
#endif
	systemHostname = Sys::hostname();;
	systemBuild = __DATE__ " " __TIME__;
	INFO("%s : %s ",Sys::hostname(),systemBuild().c_str());

	INFO("Starting nanoAkka on %s heap : %d ", Sys::getProcessor(), Sys::getFreeHeap());
	ledBlue.init();
	ledRed.init();
	wifi.init();
	mqtt.init();
	wifi.connected >> mqtt.wifiConnected;
	mqtt.connected >> poller.connected;
	mqtt.connected >> ledBlue.blinkSlow;
	//--------------------------------------------------------------------- system props
	systemUptime >> mqtt.toTopic<uint64_t>("system/upTime");
	systemHeap >> mqtt.toTopic<uint32_t>("system/heap");
	systemHostname >> mqtt.toTopic<std::string>("system/hostname");
	systemBuild >> mqtt.toTopic<std::string>("system/build");
	systemAlive >> mqtt.toTopic<bool>("system/alive");
	poller(systemUptime)(systemHeap)(systemHostname)(systemBuild)(systemAlive);
	//-----------------------------------------------------------------  WIFI props
	wifi.macAddress >> mqtt.toTopic<std::string>("wifi/mac");
	wifi.ipAddress >> mqtt.toTopic<std::string>("wifi/ip");
	wifi.ssid >> mqtt.toTopic<std::string>("wifi/ssid");
	wifi.rssi >> mqtt.toTopic<int>("wifi/rssi");
	poller(wifi.macAddress)(wifi.ipAddress)(wifi.ssid)(wifi.rssi);
//------------------------------------------------------------------- console logging

	TimerSource& logTimer=*(new TimerSource(mainThread,1,10000,true)) ;
	logTimer >> ([](const TimerMsg& tm) {
		INFO(" ovfl : %u busyPop : %u busyPush : %u threadQovfl : %u  ",stats.bufferOverflow,stats.bufferPopBusy,stats.bufferPushBusy,stats.threadQueueOverflow);
	});
	configHost == mqtt.topic<std::string>("system/host");
	configFloat == mqtt.topic<float>("system/float");
	poller(configHost)(configFloat);
//------------------------------------------------------------------- DWM1000

#ifdef TAG
	DWM1000_Tag& tag=*(new DWM1000_Tag(mainThread,
	                                   Spi::create(12,13,14,15),
	                                   DigitalIn::create(4),
	                                   DigitalOut::create(5),
	                                   sdk_system_get_chip_id() & 0xFFFF,
	                                   (uint8_t*)"ABCDEF"));
	tag.preStart();
	tag.mqttMsg >> mqtt.outgoing;
	tag.blink >> ledBlue.pulse;
	tag.blinks >> mqtt.toTopic<uint32_t>("tag/blinks");
	tag.polls >> mqtt.toTopic<uint32_t>("tag/polls");
	tag.resps >> mqtt.toTopic<uint32_t>("tag/resps");
	tag.finals >> mqtt.toTopic<uint32_t>("tag/finals");
	tag.interruptCount >> mqtt.toTopic<uint32_t>("tag/interrupts");
	tag.errs >> mqtt.toTopic<uint32_t>("tag/errs");
	tag.timeouts >> mqtt.toTopic<uint32_t>("tag/timeouts");
	poller(tag.blinks)(tag.polls)(tag.resps)(tag.finals)(tag.interruptCount)(tag.errs)(tag.timeouts);


#endif
#ifdef ANCHOR
	DWM1000_Anchor& anchor=*(new DWM1000_Anchor(mainThread,
	                         Spi::create(12,13,14,15),
	                         DigitalIn::create(4),
	                         DigitalOut::create(5),
	                         sdk_system_get_chip_id() & 0xFFFF,
	                         (uint8_t*)S(ANCHOR)));
	anchor.preStart();
	anchor.mqttMsg >> mqtt.outgoing;
	anchor.poll >> ledBlue.pulse;
	anchor.blinks >> mqtt.toTopic<uint32_t>("anchor/blinks");
	anchor.polls >> mqtt.toTopic<uint32_t>("anchor/polls");
	anchor.finals >> mqtt.toTopic<uint32_t>("anchor/finals");
	anchor.interruptCount >> mqtt.toTopic<uint32_t>("anchor/interrupts");
	anchor.errs >> mqtt.toTopic<uint32_t>("anchor/errs");
	anchor.timeouts >> mqtt.toTopic<uint32_t>("anchor/timeouts");
	anchor.distanceRef >> mqtt.toTopic<float>("anchor/distance");
	anchor.address >> mqtt.toTopic<uint16_t>("anchor/address");
	poller(anchor.blinks)(anchor.polls)(anchor.finals)(anchor.interruptCount);
	poller(anchor.errs)(anchor.timeouts)(anchor.distanceRef)(anchor.address);

#endif
	mainThread.start(); // wifi init fails if this doesn't end
	mqttThread.start();
}
