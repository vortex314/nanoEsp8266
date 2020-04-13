#ifndef CONFIGFLOW_H
#define CONFIGFLOW_H
#include <NanoAkka.h>
#define ARDUINOJSON_USE_LONG_LONG 1
#define ARDUINOJSON_ENABLE_STD_STRING 1
#include <ArduinoJson.h>
class ConfigStore {
		static bool _loaded;
	public:
		static StaticJsonDocument<1024> jsonDoc;
		static bool saveAll();
		static bool loadAll();

};

template <class T> class ConfigFlow : public Flow<T, T>, public ConfigStore  {
		std::string _name;
		T _value;
		bool ready=false;

	public:
		ConfigFlow(const char *name, T defaultValue)
			: _name(name), _value(defaultValue) {
		}

		void lazyLoad() {
			if ( ready ) return;
			loadAll();
			_value = jsonDoc[_name] | _value;
			ready=true;
		}

		void on(const T &value) {
			_value = value;
//			INFO(" key : %s",_name.c_str());
			jsonDoc[_name]=value;
			saveAll();
			request();
		}
		void request() {
			lazyLoad();
			this->emit(_value);
		}
		inline void operator=(T value) {
			on(value);
		};
		inline T operator()() {
			lazyLoad();
			return _value;
		}
};

#endif // CONFIGFLOW_H
