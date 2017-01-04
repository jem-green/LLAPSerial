//
// Modified LLAPSerial library to remove the power / sleep elemets of the class
//

#include "LLAPSerial.h"

/*
const CTable __code Commands[] = {
  {9,{'A','C','K','-','-','-','-','-','-'},cmdAck},  // must be the first entry
  {0,{'A','P','V','E','R','-','-','-','-'},cmdPVer},  // Protocol version
  {0,{'D','E','V','T','Y','P','E','-','-'},cmdDevType},  // Device Type
  {0,{'D','E','V','N','A','M','E','-','-'},cmdDevName},  // Device Name
  {0,{'H','E','L','L','O','-','-','-','-'},cmdHello},  // Echo
  {3,{'S','E','R','#','#','#','#','#','#'},cmdSer},  // Serial Number
  {0,{'$','E','R','-','-','-','-','-','-'},cmdSerReset},  // Serial Number
  {0,{'F','V','E','R','-','-','-','-','-'},cmdFVer},  // Software revision
  {7,{'C','H','D','E','V','I','D','#','#'},cmdDevID},  // Device ID
  {5,{'P','A','N','I','D','#','#','#','#'},cmdPanID},  // PANID
  {0,{'R','E','B','O','O','T','-','-','-'},cmdReset},  // reset
  {7,{'R','E','T','R','I','E','S','#','#'},cmdRetries},  // set # retrys
  {4,{'B','A','T','T','-','-','-','-','-'},cmdBatt}, // request battery voltage
  {4,{'S','A','V','E','-','-','-','-','-'},cmdSave}, // Save config to flash
#if defined(APSLEEP)  // cyclic sleep
  {0,{'I','N','T','V','L','#','#','#','#'},cmdInterval},  // SET cyclic sleep interval - 999S - three digits + timescale
                                                          // T=mS, S=S, M=mins, H=Hours, D=days
  {0,{'C','Y','C','L','E','-','-','-','-'},cmdCyclic},  // activate cyclic sleep
  {0,{'W','A','K','E','-','-','-','-','-'},cmdDeactivate},  // deactivate programmed behaviour (cyclic sleep etc)
  {5,{'W','A','K','E','C','#','#','#','-'},cmdSetSleepCount}, // set the sleep count until awake is sent
#endif
// allow all device to do a one shot sleep (including DALLAS)
  {0,{'S','L','E','E','P','#','#','#','#'},cmdActivate},  // activate Sleeping mode - one shot or sleep until interrupt
*/

void LLAPSerial::init()
{
	sMessage.reserve(10);
	bMsgReceived = false;
	deviceId[0] = '-';
	deviceId[1] = '-';
}

void LLAPSerial::init(char* dID)
{
	init();
	bMsgReceived = false;
	setDeviceId(dID);
	cMessage[12]=0;		// ensure terminated
}

void LLAPSerial::processMessage(){
	//if (LLAP.cMessage[0] != 'a') return; //not needed as already checked
	if (cMessage[1] != deviceId[0]) return;
	if (cMessage[2] != deviceId[1]) return;
	// now we have LLAP.cMessage[3] to LLAP.cMessage[11] as the actual message
	sMessage = String(&cMessage[3]); // let the main program deal with it
	bMsgReceived = true;
}

void LLAPSerial::SerialEvent()
{
	if (bMsgReceived) return; //get out if previous message not yet processed
	if (Serial.available() >= 12) {
        // get the new byte:
        char inChar = (char)Serial.peek();
        if (inChar == 'a') {
            for (byte i = 0; i<12; i++) {
                inChar = (char)Serial.read();
                cMessage[i] = inChar;
                if (i < 11 && Serial.peek() == 'a') {
                    // out of synch so abort and pick it up next time round
                    return;
                }
            }
            cMessage[12]=0;
            processMessage();
        }
        else
            Serial.read();	// throw away the character
    }
}

void LLAPSerial::sendMessage(String sToSend)
{
    cMessage[0] = 'a';
    cMessage[1] = deviceId[0];
    cMessage[2] = deviceId[1];
    for (byte i = 0; i<9; i++) {
		if (i < sToSend.length())
			cMessage[i+3] = sToSend.charAt(i);
		else
			cMessage[i+3] = '-';
    }

    Serial.print(cMessage);
    Serial.flush();
}

void LLAPSerial::sendMessage(char* sToSend)
{
	sendMessage(sToSend,NULL);
}

void LLAPSerial::sendMessage(char* sToSend, char* valueToSend)
{
    cMessage[0] = 'a';
    cMessage[1] = deviceId[0];
    cMessage[2] = deviceId[1];
    for (byte i = 0; i<9; i++) {
		if (i < strlen(sToSend))
			cMessage[i+3] = sToSend[i];
		else if (i < strlen(sToSend) + strlen(valueToSend))
			cMessage[i+3] = valueToSend[i - strlen(sToSend)];
		else
			cMessage[i+3] = '-';
    }

    Serial.print(cMessage);
    Serial.flush();
}

// This appears to enable flag memeory to be used rathe than program !

void LLAPSerial::sendMessage(const __FlashStringHelper *ifsh)
{
	sendMessage(ifsh,NULL);
}

// Send message with padding -

void LLAPSerial::sendMessage(const __FlashStringHelper *ifsh, char* valueToSend)
{
	const char PROGMEM *p = (const char PROGMEM *)ifsh;
	byte eos = 0;
    cMessage[0] = 'a';
    cMessage[1] = deviceId[0];
    cMessage[2] = deviceId[1];
    for (byte i = 0; i<9; i++) {
		if (!eos)
		{
			cMessage[i+3] = pgm_read_byte(p++);
			if (!cMessage[i+3]) // end of string
			{
				eos = i-3;
			}
		}
		if (eos)
		{
			if (i < eos + strlen(valueToSend))
				cMessage[i+3] = valueToSend[i - eos];
			else
				cMessage[i+3] = '-';
		}
    }
    Serial.print(cMessage);
    Serial.flush();
}

void LLAPSerial::sendInt(String sToSend, int value)
{
	char cValue[7];		// long enough for -32767 and the trailing zero
	itoa(value, cValue,10);
	byte cValuePtr = 0;

    cMessage[0] = 'a';
    cMessage[1] = deviceId[0];
    cMessage[2] = deviceId[1];
    for (byte i = 0; i<9; i++) {
		if (i < sToSend.length())
			cMessage[i+3] = sToSend.charAt(i);
		else if (cValuePtr < 7 && cValue[cValuePtr] !=0)
			cMessage[i+3] = cValue[cValuePtr++];
		else
			cMessage[i+3] = '-';
    }

    Serial.print(cMessage);
    Serial.flush();
}

void LLAPSerial::sendIntWithDP(String sToSend, int value, byte decimalPlaces)
{
	char cValue[8];		// long enough for -3276.7 and the trailing zero
	byte cValuePtr=0;
	itoa(value, cValue,10);
	char* cp = &cValue[strlen(cValue)];
	*(cp+1) = 0;	// new terminator
	while (decimalPlaces-- && --cp )
	{
		*(cp+1) = *cp;
	}
	*cp = '.';

    cMessage[0] = 'a';
    cMessage[1] = deviceId[0];
    cMessage[2] = deviceId[1];
    for (byte i = 0; i<9; i++) {
		if (i < sToSend.length())
			cMessage[i+3] = sToSend.charAt(i);
		else if (cValuePtr < 8 && cValue[cValuePtr] !=0)
			cMessage[i+3] = cValue[cValuePtr++];
		else
			cMessage[i+3] = '-';
    }

    Serial.print(cMessage);
    Serial.flush();
}

void LLAPSerial::setDeviceId(char* cId)
{
    deviceId[0] = cId[0];
    deviceId[1] = cId[1];
}
