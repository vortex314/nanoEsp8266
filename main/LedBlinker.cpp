#include "LedBlinker.h"


LedBlinker::LedBlinker(Thread& thr,uint32_t pin, uint32_t delay)
    : Actor(thr),_ledGpio(DigitalOut::create(pin)),blinkTimer(thr,BLINK_TIMER_ID,delay,true)
{

    blinkTimer >> ([&](const TimerMsg tm) {
        _ledGpio.write(_on);
        _on = _on ? 0 : 1 ;
    });

    blinkSlow.async(thread(),[&](bool flag) {
        if ( flag ) blinkTimer.interval(500);
        else blinkTimer.interval(100);
    });
}
void LedBlinker::init()
{
    _ledGpio.init();
    _ledGpio.write(1);
    _on=0;
}

void LedBlinker::delay(uint32_t d)
{
    blinkTimer.interval(d);
}

void LedBlinker::pulse()
{
    blinkTimer.repeat(false);
    _ledGpio.write(0);
    _on=1;
    blinkTimer.start();
}
