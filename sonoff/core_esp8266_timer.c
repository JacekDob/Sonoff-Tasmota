/*
  timer.c - Timer1 library for esp8266

  Copyright (c) 2015 Hristo Gochkov. All rights reserved.
  This file is part of the esp8266 core for Arduino environment.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "wiring_private.h"
#include "pins_arduino.h"

#ifdef ESP8266
#include "c_types.h"
#include "ets_sys.h"
#endif

#ifdef ESP32
#define timercallback void*
#endif


// ------------------------------------------------------------------ -
// timer 1

static volatile timercallback timer1_user_cb = NULL;

void ICACHE_RAM_ATTR timer1_isr_handler(void *para){
#ifdef ESP8266
    (void) para;
    if ((T1C & ((1 << TCAR) | (1 << TCIT))) == 0) TEIE &= ~TEIE1;//edge int disable
    T1I = 0;
    if (timer1_user_cb) {
        // to make ISR compatible to Arduino AVR model where interrupts are disabled
        // we disable them before we call the client ISR
        uint32_t savedPS = xt_rsil(15); // stop other interrupts
        timer1_user_cb();
        xt_wsr_ps(savedPS);
    }
#endif    
}

void ICACHE_RAM_ATTR timer1_isr_init(){
#ifdef ESP8266    
    ETS_FRC_TIMER1_INTR_ATTACH(timer1_isr_handler, NULL);
#endif    
}

void timer1_attachInterrupt(timercallback userFunc) {
#ifdef ESP8266  
    timer1_user_cb = userFunc;
    ETS_FRC1_INTR_ENABLE();
#endif    
}

void ICACHE_RAM_ATTR timer1_detachInterrupt() {
#ifdef ESP8266
    timer1_user_cb = 0;
    TEIE &= ~TEIE1;//edge int disable
    ETS_FRC1_INTR_DISABLE();
#endif    
}

void timer1_enable(uint8_t divider, uint8_t int_type, uint8_t reload){
#ifdef ESP8266
    T1C = (1 << TCTE) | ((divider & 3) << TCPD) | ((int_type & 1) << TCIT) | ((reload & 1) << TCAR);
    T1I = 0;
#endif    
}

void ICACHE_RAM_ATTR timer1_write(uint32_t ticks){
#ifdef ESP8266
    T1L = ((ticks)& 0x7FFFFF);
    if ((T1C & (1 << TCIT)) == 0) TEIE |= TEIE1;//edge int enable
#endif    
}

void ICACHE_RAM_ATTR timer1_disable(){
#ifdef ESP8266
    T1C = 0;
    T1I = 0;
#endif
}

//-------------------------------------------------------------------
// timer 0

static volatile timercallback timer0_user_cb = NULL;

void ICACHE_RAM_ATTR timer0_isr_handler(void* para){
#ifdef ESP8266  
    (void) para;
    if (timer0_user_cb) {
        // to make ISR compatible to Arduino AVR model where interrupts are disabled
        // we disable them before we call the client ISR
        uint32_t savedPS = xt_rsil(15); // stop other interrupts
        timer0_user_cb();
        xt_wsr_ps(savedPS);
    }
#endif    
}

void timer0_isr_init(){
#ifdef ESP8266  
    ETS_CCOMPARE0_INTR_ATTACH(timer0_isr_handler, NULL);
#endif    
}

void timer0_attachInterrupt(timercallback userFunc) {
#ifdef ESP8266  
    timer0_user_cb = userFunc;
    ETS_CCOMPARE0_ENABLE();
#endif    
}

void ICACHE_RAM_ATTR timer0_detachInterrupt() {
#ifdef ESP8266    
    timer0_user_cb = NULL;
    ETS_CCOMPARE0_DISABLE();
#endif    
}
