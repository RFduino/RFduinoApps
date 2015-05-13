//
// Copyright (c) 2015, RF Digital Corp.
// All rights reserved.

// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//    1. Redistributions of source code must retain the above copyright notice,
//       this list of conditions and the following disclaimer.
//
//    2. Redistributions in binary form must on an RF Digital part and reproduce the 
//       above copyright notice, this list of conditions and the following disclaimer 
//       in the documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
// WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE.
//
#include "Arduino.h"
#include "RFduino_Timer2.h"
#include <assert.h>

#include <functional>   // std::bind


void timer2Interupt(void);

RFduino_Timer2::RFduino_Timer2(void)
{
	_timerExpiresCallback = NULL;
	_timerCallbackUserData = NULL;

	_timerBExpiresCallback = NULL;
	_timerBCallbackUserData = NULL;

	_prescaler = 9;

	// Start 32 MHz crystal oscillator
	NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
	NRF_CLOCK->TASKS_HFCLKSTART = 1;
	// Wait for the external oscillator to start up
	while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0) {
		// Do nothing.
	}
	
	//NVIC_EnableIRQ(TIMER2_IRQn);
}

void RFduino_Timer2::interval(unsigned int ms)
{
	float hz = 16000000.0 / pow(2, _prescaler);
	float tn = (1.0 / hz); // uS per tick
	float onems = 1.0 / (tn * 1000);
	int newval = (ms * 31) + (ms / 4);
	
	assert(onems * ms < pow(2, 16));
	_interval = round(onems * ms);
}

void RFduino_Timer2::intervalB(unsigned int ms)
{
	float hz = 16000000.0 / pow(2, _prescaler);
	float tn = (1.0 / hz); // uS per tick
	float onems = 1.0 / (tn * 1000);
	int newval = (ms * 31) + (ms / 4);

	assert(onems * ms < pow(2, 16));
	_intervalB = round(onems * ms);
}

void RFduino_Timer2::prescaler(unsigned int prescaler)
{
	assert(prescaler < 10);
	_prescaler = prescaler;
}

unsigned int RFduino_Timer2::prescaler(void)
{
	return _prescaler;
}

void RFduino_Timer2::start(void)
{
	configureTimer();
	NRF_TIMER2->TASKS_START = 1;
}

void RFduino_Timer2::stop(void)
{
	NRF_TIMER2->TASKS_STOP = 1;
}

void RFduino_Timer2::configureTimer(void)
{
	NRF_TIMER2->TASKS_STOP = 1;	                
	NRF_TIMER2->MODE = TIMER_MODE_MODE_Timer;                   
	NRF_TIMER2->TASKS_CLEAR = 1;                                
	NRF_TIMER2->BITMODE = TIMER_BITMODE_BITMODE_16Bit;               
	NRF_TIMER2->PRESCALER = _prescaler;	                                   

	NRF_TIMER2->INTENSET = (TIMER_INTENSET_COMPARE0_Enabled << TIMER_INTENSET_COMPARE0_Pos) | (TIMER_INTENSET_COMPARE1_Enabled << TIMER_INTENSET_COMPARE1_Pos);
	NRF_TIMER2->SHORTS = (TIMER_SHORTS_COMPARE0_CLEAR_Enabled << TIMER_SHORTS_COMPARE0_CLEAR_Pos);                           
	
	NRF_TIMER2->CC[0] = _interval;									 
	NRF_TIMER2->CC[1] = _intervalB;                                 
	attachInterrupt(TIMER2_IRQn, RFduino_Timer2::_interuptCB);	

}

void RFduino_Timer2::setOnTimerExpires(RFduino_Timer2CallBackType callback, void* user_data)
{
	_timerExpiresCallback = callback;
	_timerCallbackUserData = user_data;
}

void RFduino_Timer2::setOnTimerBExpires(RFduino_Timer2CallBackType callback, void* user_data)
{
	_timerBExpiresCallback = callback;
	_timerBCallbackUserData = user_data;
}

void RFduino_Timer2::_interuptCB(void)
{
	if ((NRF_TIMER2->EVENTS_COMPARE[0] != 0) && ((NRF_TIMER2->INTENSET & TIMER_INTENSET_COMPARE0_Msk) != 0))
	{
		if (timer2->_timerExpiresCallback != NULL)
			timer2->_timerExpiresCallback(timer2->_timerCallbackUserData);
		NRF_TIMER2->EVENTS_COMPARE[0] = 0;
	}
	if ((NRF_TIMER2->EVENTS_COMPARE[1] != 0) && ((NRF_TIMER2->INTENSET & TIMER_INTENSET_COMPARE1_Msk) != 0))
	{
		if (timer2->_timerBExpiresCallback != NULL)
			timer2->_timerBExpiresCallback(timer2->_timerBCallbackUserData);
		NRF_TIMER2->EVENTS_COMPARE[1] = 0;
	}
}

RFduino_Timer2 *timer2 = new RFduino_Timer2();