//
// Copyright (c) 2015, RF Digital Corp.
// All rights reserved.
//
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
// A simple library for the Timer2 on the RFduino module. The registered
// callback function is called when the timer expires. The timer can also
// be configured via the library
//
#ifndef RFduino_Timer2_h
#define RFduino_Timer2_h

#include "Arduino.h"

typedef void(*RFduino_Timer2CallBackType)(void* user_data);

class RFduino_Timer2
{
   public:
	   void interval(unsigned int ms);
	   void intervalB(unsigned int ms);
	   void prescaler(unsigned int prescaler);
		unsigned int prescaler(void);
		void start(void);
		void stop(void);
		void setOnTimerExpires(RFduino_Timer2CallBackType callback, void* user_data);
		void setOnTimerBExpires(RFduino_Timer2CallBackType callback, void* user_data);
		RFduino_Timer2(void);

   private:
	    int _prescaler;
		unsigned int _interval;
		unsigned int _intervalB;
		void configureTimer(void);
		static void _interuptCB(void);
		RFduino_Timer2CallBackType _timerExpiresCallback;
		RFduino_Timer2CallBackType _timerBExpiresCallback;
		void *_timerCallbackUserData;
		void *_timerBCallbackUserData;
};

extern RFduino_Timer2 *timer2;

#endif
