// arduino usb port at 'dmesg | grep "tty"'

#include <Arduino.h>
#include "AirRide.h"
#include <time.h>
#include <string.h>
#include <stdlib.h>

#define NUMSHOCKS 4
Shock *shocks[NUMSHOCKS];
Compressor *cmp;

// I use this a lot so its easier to define it
// used for storing constant strings in flash instead of ram
// using char array instead of strings since it saves more space
#define STRCONST const static char str[] PROGMEM

// any function definitions used in main
// cannot use cli(), that is an interrupts function
void aircli();

void setup()
{
	// use defined BAUD_R
	Serial.begin(BAUD_R); // enable serial for debug
	while (!Serial)
		delay(10);

	// TODO: integrate i2c boards to get shock inSerial.println
	for (int i = 0; i < NUMSHOCKS; i++)
		shocks[i] = new Shock(SHOCK_VALVE_PIN_START + i, SHOCK_HEIGHT_PIN_START + i, SHOCK_PRESSURE_PIN_START + i);
	cmp = new Compressor(COMPRESSOR_MOTOR_PIN, COMPRESSOR_PRESSURE_PIN);
}

void loop()
{
	//cmp1->update();  // main program loop for Compressor
	//cmp1->isFilling();
	aircli();

	// update all hardware devices
	for (int i = 0; i < 4; i++)
		shocks[i]->update();
	cmp->update();

	// TODO: remove all delays and replace with oneshot flasher bits
	delay(10);
}

/**
 * checkserial uses C functions for string processing, for speed
 * Serial buffer handling is using builtin cpp arduino libraries
 * 
 * checks serial buffer for commands
 * if commands available, then separate into arguements
 * memory for buffer and arguements is dynamic, for memory savings
 */
void aircli()
{
	if (Serial.available() > 0)
	{
		// need to malloc space for buffer
		// use local memory, so life is easier
		static uint16_t bufsize = 0; // starts as 0
		const uint8_t bufstep = 32;	 // step buffer sizes 32 bytes at a time
		static char *buf = NULL;	 // initialized as null
		// TODO: make sure serial messages from pi end with \n,
		// to accurately separate commands when sent in fast succession
		int i = 0;
		while (Serial.available() > 0) // process response
		{
			if (bufsize >= 256)
			{
				STRCONST = "cli buffer overflow!";
				debug_log(str);
				exit(-1); // kill processor
			}
			if (i >= bufsize - 1) // if not enough space in buffer, increase size
				buf = (char *)realloc(buf, bufsize += bufstep);
			buf[i] = Serial.read();
			// newline is an indicator of end of message
			// used for quick, concurrent messages
			if (buf[i] == '\n')
				break;
			i++;
		}
		buf[i] = '\0'; // ENSURE null terminator is there
		// printf("message in buf is: %s\n", buf);
		if (i < bufsize - bufstep) // decrease to the smallest block that fits buffer
			buf = (char *)realloc(buf, (bufsize = ((i / bufstep) + 1) * bufstep));
		Serial.println(buf); // echo commands back
		// use c string functions, since they are faster than objects
		static int argc = 0;
		static char **argv = NULL;
		if (argc == 0) // first check, set up arguement handler
			argv = (char **)malloc((argc = 1) * sizeof(char *));

		argv[0] = strtok(buf, " "); // set first string
		char *inpstr;
		int x = 1;
		while (inpstr = strtok(NULL, " ")) // split string into separate args
		{
			x++;
			if (x > argc)													// if need more space for another arg
				argv = (char **)realloc(argv, (argc = x) * sizeof(char *)); // need to make sure there
			argv[x - 1] = inpstr;
		}
		if (x < argc) // readjust for new number of args
			argv = (char **)realloc(argv, (argc = x) * sizeof(char *));

		/********************************************************************/

		// now that we have the arguements separated, we can process them

		// first arg is command specifier
		if (!strcmp(argv[0], "shock")) // shock commands
		{
			int x;
			if (!strcmp(argv[1], "all"))
				x = -1;
			else
				sscanf(argv[1], "%d", &x); // convert arg to int

			// now that we know which shock, next arg will be either get or set
			if (!strcmp(argv[2], "set")) // set shock setting
			{
				// find which item to set
				float setpoint;
				sscanf(argv[4], "%f", &setpoint);
				if (!strcmp(argv[3], "pressure"))
				{
					if (x < 0)
						for (int i = 0; i < 4; i++)
							shocks[i]->setPressure(setpoint);
					else
						shocks[i]->setPressure(setpoint);
					debug_log("set shock " + String(argv[1]) + " pressure to " + String(setpoint) + "psi");
				}
				else if (!strcmp(argv[3], "height"))
				{
					if (x < 0)
						for (int i = 0; i < 4; i++)
							shocks[i]->setHeight(setpoint);
					else
						shocks[i]->setHeight(setpoint);
					debug_log("set shock " + String(argv[1]) + " height to " + String(setpoint) + "mm");
				}
				else
					debug_log("Unrecognized shock set arguement");
			}
			else if (!strcmp(argv[2], "get"))
			{
				// find which item to get
				if (!strcmp(argv[3], "pressure"))
				{
					if (x < 0)
						for (int i = 0; i < 4; i++)
						{
							float temp = shocks[i]->getPressure();
							Serial.println(String(temp));
							debug_log("got shock " + String(i) + " pressure: " + String(temp) + "psi");
						}
					else
					{
						float temp = shocks[x]->getPressure();
						Serial.println(String(temp));
						debug_log("got shock " + String(x) + " pressure: " + String(temp) + "psi");
					}
				}
				else if (!strcmp(argv[3], "height"))
				{
					if (x < 0)
						for (int i = 0; i < 4; i++)
						{
							float temp = shocks[i]->getHeight();
							Serial.println(String(temp));
							debug_log("got shock " + String(i) + " height: " + String(temp) + "mm");
						}
					else
					{
						float temp = shocks[x]->getHeight();
						Serial.println(String(temp));
						debug_log("got shock " + String(x) + " height: " + String(temp) + "mm");
					}
				}
				else
					debug_log("Unrecognized shock 'get' arguement");
			}
			else
				debug_log("Unrecognized shock arguement, must use set/get/help");
		}
		else if (!strcmp(argv[0], "compressor"))
		{
			/**
			 * 'compressor' commands:
			 * set: set 'run' or 'idle' pressure, float psi
			 * get: get 'run', 'idle', or 'current' setpoint pressure, float psi, or specify 'state' to get run mode <running, idling, stopped>
			 * start: start compressor
			 * stop: stop compressor running
			 * idle: start compressor in idle mode
			 * bleed: open bleed valve and drop pressure to 0
			 */
			// now that we know which shock, next arg will be either get or set
			// set: set 'run' or 'idle' pressure, float psi
			if (!strcmp(argv[1], "set")) // set shock setting
			{
				// find which item to set
				float setpoint;
				sscanf(argv[3], "%f", &setpoint); // compressor set run/idle [setpoint]
				if (!strcmp(argv[3], "run"))
				{
					cmp->setPressure(setpoint);
					debug_log("set compressor running pressure to " + String(setpoint) + "psi");
				}
				else if (!strcmp(argv[3], "idle"))
				{
					cmp->setIdlePressure(setpoint);
					debug_log("set compressor idling pressure to " + String(setpoint) + "psi");
				}
				else
					debug_log("Unrecognized compressor set arguement");
			}
			// get: get 'run', 'idle', or 'current' setpoint pressure, float psi, or specify 'state' to get run mode <running, idling, stopped>
			else if (!strcmp(argv[2], "get"))
			{
				// find which item to get
				if (!strcmp(argv[3], "run"))
				{
					Serial.println(String(cmp->_runPres));
				}
				else if (!strcmp(argv[3], "idle"))
				{
					Serial.println(String(cmp->_idlePres));
				}
				else if (!strcmp(argv[3], "current"))
				{
					Serial.println(String(cmp->getPressure()));
				}
				else if (!strcmp(argv[3], "state"))
				{
					switch (cmp->getState())
					{
					case 0:
						Serial.println("stopped");
						break;
					case 1:
						Serial.println("running");
						break;
					case 2:
						Serial.println("idling");
						break;
					default:
						STRCONST = "FAULT: unknown compressor mode";
						Serial.println(str);
					}
				}
				else
				{
					STRCONST = "Unrecognized shock 'get' arguement";
					debug_log(str);
				}
			}
			// start: start compressor
			else if (!strcmp(argv[1], "start"))
			{
				cmp->start();
				STRCONST = "Compressor started in run mode, regulating pressure at ";
				debug_log(str + String(cmp->_runPres) + "psi.");
			}
			// stop: stop compressor running
			else if (!strcmp(argv[1], "stop"))
			{
				cmp->stop();
				STRCONST = "Compressor stopped regulating pressure.";
				debug_log(str);
			}
			// idle: start compressor in idle mode
			else if (!strcmp(argv[1], "idle"))
			{
				cmp->start_idle();
				STRCONST = "Compressor started in idle mode, regulating pressure at ";
				debug_log(str + String(cmp->_idlePres) + "psi.");
			}
			// bleed: open bleed valve and drop pressure to 0
			else if (!strcmp(argv[1], "bleed"))
			{
				// TODO: compressor needs a bleed function
				// cmp->bleed();
				STRCONST = "Bleeding compressor";
				debug_log(str);
				debug_log("NO BLEED FUNCTION MADE YET");
			}
			else
			{
				STRCONST = "Unrecognized shock arguement, must use set/get/help";
				debug_log(str);
			}
		}
		else if (!strcmp(argv[0], "help"))
		{
			// TODO: finish help command
			if (argc == 1)
			{
				// TODO: just 'help' command response here
			}
			else if (!strcmp(argv[1], "shock")) // "help shock" response
			{
				// help will display how to use shock
				// TODO: implement more shock commands
				STRCONST = "usage: shock [0..3 / all] get/set pressure/height [setpoint if 'set']\n\
				setting pressure or height will also set system to that mode\n\
				'all' sets all shocks to same setting/mode";
				Serial.println(str);
			}
			else if (!strcmp(argv[1], "compressor"))
			{
				STRCONST = "compressor commands: \n\
				 set: set 'run' or 'idle' pressure (float psi), usage: compressor set <run/idle> <psivalue>\n\
				 get 'run', 'idle', or 'current' setpoint pressure, or specify 'state' to get run mode <running, idling, stopped>\n\
				 start: start compressor holding run pressure\n\
				 stop: stop compressor cycle\n\
				 idle: start compressor in idle mode\n\
				 bleed: open bleed valve and drop pressure to 0";
				Serial.println(str);
			}
			else
			{
				STRCONST = "Sorry, help command is not yet implemented.";
				debug_log(str);
			}
		}
		else
		{
			STRCONST = "unrecognized serial command. type 'help' for more info";
			Serial.println(str);
		}
	}
}
