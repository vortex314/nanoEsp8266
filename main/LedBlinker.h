#ifndef LEDBLINKER_H
#define LEDBLINKER_H

#include <NanoAkka.h>
#include <Hardware.h>

class LedBlinker : public Actor {
		int _on=0;
		DigitalOut& _ledGpio;

	public:
		static const int BLINK_TIMER_ID=1;
		TimerSource blinkTimer;
		Sink<TimerMsg,4> timerHandler;
		Sink<bool,4> blinkSlow;
		LedBlinker(Thread& thr,uint32_t pin, uint32_t delay);
		void init();
		void delay(uint32_t d);
		void onNext(const TimerMsg&);
        void pulse();
};

#endif // LEDBLINKER_H
