/*
   RFduino_Timer2.h - Library for flashing Morse code.
   Released into the public domain.
 */
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
