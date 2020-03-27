#ifndef WIFI_H
#define WIFI_H

#include <Log.h>
#include <NanoAkka.h>
#include "espressif/esp_common.h"
#include <FreeRTOS.h>
#include <espressif/esp_sta.h>
#include <espressif/esp_wifi.h>
#include <espressif/esp_system.h>
#include <lwip/api.h>
#include <lwip/netif.h>
#ifndef WIFI_SSID
#error "WIFI_SSID should be defined !"
#endif

#ifndef WIFI_PASS
#error "WIFI_PASS should be defined !"
#endif

class Wifi {
		uint8_t _mac[6];
		static Wifi* _wifi;
		bool _foundAP=false;
		enum { INIT, SCANNING, START_CONNECT,CONNECTING, CONNECTED } _state=INIT;
		struct sdk_station_config _config;
		uint32_t _retries;
		uint8_t _status;


	public:
		TimerSource checkTimer;
		ValueSource<bool> connected=false;
		ValueSource<int> rssi;
		ValueSource<std::string> ipAddress;
		ValueSource<std::string> ssid;
		ValueSource<uint64_t> mac;
		ValueSource<std::string> macAddress;
		ValueSource<std::string> password;
		ValueSource<std::string> prefix;

		Wifi( Thread& thr);
		void init();
//		bool scanDoneHandler();
		void connectToAP(const char* AP);
		void startScan();

		static void scan_done_cb(void* arg, sdk_scan_status_t status);
		void scan();
		bool isConnected();
		void becomeConnected();
};

#endif // WIFI_H
