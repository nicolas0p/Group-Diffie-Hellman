/*
*	THIS CLASS CONTAINS METHODS TO GIVE THE USER A FEEDBACK OF WHAT'S HAPPENING
*	
----------------WHEN THE CODE IS ABOUT TO GO OUT OF THE LAB, SET [isProduction] attribute to true
*
* --> it uses the board led on GPIO C3 to gove some feedback
* --> it uses the USB interface to print feedback messages

----------------METHODS
*
* --> showLife(): returns nothing and receive nothing. Will blink the LED to show that the board is working
* --> blinkSuccess(...): receives the type of success feedback as parameter and then blink the LED and print message
* --> blinkError(...): same as blinkSuccess but receives a error type
* --> printMessage(...): print a message, receives the type and the data to be printed if needed
* --> printSendingData(...): receives the data got from the sensors and print on the screen
* --> printSentMessage(...): receives the string that is going to be sent over the network and print it on the screen
* --> blink(...): receives the amount of blinkings and period between them
*/


#ifndef INTERFACE_H_
#define INTERFACE_H_

#include <machine.h>
#include <gpio.h>
#include <machine/cortex_m/emote3_gptm.h>

using namespace EPOS;

class Interface{
	public:		
		enum ERROR{NOGPRSCARD,NONETWORK,TRYINGSENDAGAIN};
		enum SUCCESS{SIMCARDINITIALIZED,MESSAGESENT};
		enum MESSAGE{GPRSCREATED,
				GPRSFAIL,
				NETWORKFAIL,
				STARTINGNETWORK,
				SENDCOMMAND,
				WAITINGRESPONSE,
				SENDDATARESULT,
				GPRSSETUP,
				GPRSINITIALIZED,
				GPRSSTATUS,
				GPRSSENDERROR,
				CHECKINGFLASH,
				GETTINGFLASHDATA,
				MESSAGESNOTSENT,
				ALLMESSAGESSENT
				};

		Interface(){
			led = new EPOS::GPIO{'c', 3, EPOS::GPIO::OUTPUT};
		}

		void showLife();

		void blinkSuccess(SUCCESS type);
		void blinkError(ERROR type);
		static void printMessage(MESSAGE m,int data);
		static void printSendingData(unsigned int data[]);
		static void printSentMessage(const char msg[]);

		static const bool isProduction = false;

	private:		
		GPIO *led;

		void blink(unsigned int x,unsigned int period = 50000);

};

#endif

