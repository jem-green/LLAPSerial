// LLAPSerial.h

#ifndef _LLAPSERIAL_h
#define _LLAPSERIAL_h

	#if defined(ARDUINO) && ARDUINO >= 100
		#include "Arduino.h"
	#else
		#include "WProgram.h"
	#endif

	class LLAPSerial
	{
		private:
			char cMessage[13];
			void processMessage();
		public:
			void init();
			void init(char* cI);
			char deviceId[2];
			String sMessage;
			boolean bMsgReceived;
			void SerialEvent();
			void sendMessage(String sToSend);
			void sendMessage(String sToSend, char* valueToSend);
			void sendMessage(char* sToSend);
			void sendMessage(char* sToSend, char* valueToSend);
			void sendMessage(const __FlashStringHelper *ifsh);
			void sendMessage(const __FlashStringHelper *ifsh, char* valueToSend);
			void sendInt(String sToSend, int value);
			void sendIntWithDP(String sToSend, int value, byte decimalPlaces);
			void sendIntWithPad(String sToSend, int value, byte length);
			void sendIntWithTerminator(String sToSend, int value, byte length, char terminator);
			void setDeviceId(char* cId);
	};

#endif

