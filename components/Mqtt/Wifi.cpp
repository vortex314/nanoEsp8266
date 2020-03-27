#include <Wifi.h>



Wifi* Wifi::_wifi = 0; // system sigleton, needed in callback

Wifi::Wifi(Thread& thr) : checkTimer(thr,1,3000,true), password(S(WIFI_PASS)),prefix(S(WIFI_SSID)) {
	_wifi = this;
	_foundAP = false;
	_state = INIT;
};

void Wifi::connectToAP(const char* ssid) {
	sdk_wifi_station_disconnect();
	netif_set_hostname(netif_default, Sys::hostname());
	sdk_wifi_set_opmode(STATION_MODE);
	sdk_wifi_station_set_config(&_config);
	sdk_wifi_station_connect();
}


void Wifi::init() {
	_state = INIT;
	ZERO(_config);
	strcpy((char*)_config.password, password().c_str());
	if(sdk_wifi_get_opmode() == SOFTAP_MODE) { INFO("ap mode can't scan !!!\r\n"); }

	checkTimer >> ([&](const TimerMsg& tm ) {
		switch(_state) {
			case INIT : {
					connected=false;
					_foundAP=false;
					_state = SCANNING;
					_retries=20;
					if ( ! sdk_wifi_set_opmode(STATION_MODE) )	WARN("sdk_wifi_set_opmode(STATION_MODE) failed.");
					if ( !sdk_wifi_station_scan(NULL, scan_done_cb) )	WARN("sdk_wifi_station_scan() failed.");
					break;
				}

			case SCANNING: {
					if(_foundAP) {
						INFO(" Scan end, ssid : %s ",ssid().c_str());
						strcpy((char*)_config.ssid, ssid().c_str());
						sdk_wifi_station_disconnect();
						_retries=10;
						connectToAP(ssid().c_str());
						_state = CONNECTING;
					} else {
						INFO("Wait for scan end... %d ",_retries--);
						if ( _retries==0) {
							_state=INIT;
							INFO(" restarting scan ");
						}
					}
					break;
				}
			case CONNECTING : {
					--_retries;
					if(_retries == 0) {
						_state=INIT;
					}
					_status = sdk_wifi_station_get_connect_status();
					INFO("Wifi status =%d , retries left: %d", _status, _retries);
					if(_status == STATION_WRONG_PASSWORD) {
						INFO("WiFi: wrong password");
					} else if(_status == STATION_NO_AP_FOUND) {
						INFO("WiFi: AP not found");
					} else if(_status == STATION_CONNECT_FAIL) {
						INFO("WiFi: connection failed");
					} else if ( _status == STATION_GOT_IP ) {
						ip_info ipInfo;
						if(sdk_wifi_get_ip_info(STATION_IF, &ipInfo)) {
							ipAddress = ip4addr_ntoa((ip4_addr_t*)&ipInfo.ip.addr);
							INFO(" ip info %s", ipAddress().c_str());
						}
						_state=CONNECTED;
						connected=true;
					} else if ( _status == STATION_IDLE ) {
						INFO("WiFi: STATION_IDLE");
					} else if ( _status == STATION_CONNECTING ) {
						INFO("WiFi: STATION_CONNECTING");
					} else {
						WARN(" unknown wifi state: %d",_status);
					}

					break;
				}
			case CONNECTED : {
					_status = sdk_wifi_station_get_connect_status();
					if ( _status == STATION_GOT_IP) {
						if ( ! connected()) connected=true;
					} else {
						connected=false;
						INFO("disconnected...");
						sdk_wifi_station_disconnect();
						_state=INIT;
					}
					break;
				}

			default : {
					WARN(" unexpected state : %d ",_state);
				}
		}
	});


	union {
		uint8_t macBytes[13];
		uint64_t mac;
	} un;
	sdk_wifi_get_macaddr(STATION_IF, (uint8_t *)un.macBytes);
	std::string macString;
	string_format(macString,"%02X:%02X:%02X:%02X:%02X:%02X",un.macBytes[0],un.macBytes[1],un.macBytes[2],un.macBytes[3],un.macBytes[4],un.macBytes[5]);
	macAddress = macString;
	mac = un.mac;
}


static const char* const auth_modes[] = { [AUTH_OPEN] = "Open",
                                          [AUTH_WEP] = "WEP",
                                          [AUTH_WPA_PSK] = "WPA/PSK",
                                          [AUTH_WPA2_PSK] = "WPA2/PSK",
                                          [AUTH_WPA_WPA2_PSK] = "WPA/WPA2/PSK"
                                        };

void Wifi::scan_done_cb(void* arg, sdk_scan_status_t status) {
	char ssid[33]; // max SSID length + zero byte
	static std::string ssidStr;

	INFO("scan_done_cb()");

	if(status != SCAN_OK) {
		WARN("Error: WiFi scan failed\n");
		return;
	}

	struct sdk_bss_info* bss = (struct sdk_bss_info*)arg;
	// first one is invalid
	bss = bss->next.stqe_next;

	struct sdk_bss_info* strongestAP = 0;
	int strongestRssi = -1000;
	while(NULL != bss) {
		size_t len = strlen((const char*)bss->ssid);
		memcpy(ssid, bss->ssid, len);
		ssid[len] = 0;
		ssidStr = ssid;
		INFO("%16s (" MACSTR ") RSSI: %02d, security: %s", ssid, MAC2STR(bss->bssid), bss->rssi,
		     auth_modes[bss->authmode]);
		if(ssidStr.find(_wifi->prefix()) == 0 && bss->rssi > strongestRssi) {
			strongestAP = bss;
			strongestRssi = bss->rssi;
		}
		bss = bss->next.stqe_next;
	}
	if(strongestAP) {
		_wifi->ssid = (const char*)strongestAP->ssid;
		_wifi->rssi = strongestAP->rssi;
		_wifi->_foundAP = true;
		INFO(" selected AP : %s ", _wifi->ssid().c_str());
		return;
	}
}
