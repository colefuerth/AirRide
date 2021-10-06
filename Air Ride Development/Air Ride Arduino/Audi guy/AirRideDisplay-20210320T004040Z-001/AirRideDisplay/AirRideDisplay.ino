/*
    <AirRideDisplay.ino Reads and distributes data to an lcd>
    Copyright (C) <2013>  <Joseph E Cannon>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


/*///////////////////////////////////
...................................//
..............._____ ..............//
........______//__{\______ ........//
....../__(O)__//___/__(O)_/........//
...................................//
								   //
 * AirRideDisplay.ino			   //
 *								   //
 * Author: Joseph E Cannon		   //
 * Created: 10/1/2013			   //
 *								   //
...................................//
..............._____ ..............//
........______//__{\______ ........//
....../__(O)__//___/__(O)_/........//
...................................//
*////////////////////////////////////

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <avr/eeprom.h>
#include <Button.h>
#include "AirRideDisplay.h"
#include "DisplayStrings.h"

void setup()
{ 
#ifdef DEBUG
	//erraseEEPROMZeros();
	Serial.begin(9600);
#endif
	/*
	Serial.begin(9600);
	int temp = settings.rearRFlag ? 100 : 0;
	Serial.println(temp);

	settings.rearRFlag = true;

	temp = settings.rearRFlag ? 0 : 100;
	Serial.println(temp);

	temp = settings.rearRFlag == true ? 100 : 0;
	Serial.println(temp);

	temp = settings.rearRFlag == false ? 100 : 0;
	Serial.println(temp);
	*/
	
	//Load settings. Save defualt settings if non exist.
	loadSettings();

	//initialize LCD panel
	lcd.begin (20,4);
  
	// Switch on the backlight
	lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);
  
	if(settings.lcdBacklightON == 1)
		lcd.setBacklight(HIGH);
	else
		lcd.setBacklight(LOW);
	
}

void loop()
{ 
	
	//Check to see if we've run the setup program yet. If not we need to run it.
	butState = getInputState();

	if(butState == toggleBacklight)
	{
		settings.lcdBacklightON = !settings.lcdBacklightON;

		if(settings.lcdBacklightON)
			lcd.setBacklight(HIGH);
		else
			lcd.setBacklight(LOW);

		saveSettings();
	}

	//
	//
	switch (ARDstate)
	{
	case dsSetup:
		SetupMode();
		break;
	case dsNormal:
		NormalMode();
		break;
	case dsAnimation:
		AnimationMode();
		break;
	case dsError:
		ErrorMode();
		break;
	default:
		break;
   }
   
}

//SetupMode()
void SetupMode()
{
	switch (setupState)
	{
	case enterSetup:
		lcd.clear();
		lcd.home ();
		lcd.print(step1Line1);
		lcd.setCursor ( 0, 1 );
		lcd.print(step1Line2);
		lcd.setCursor ( 0, 2 );
		lcd.print(step1Line3);
		lcd.setCursor ( 0, 3 );
		lcd.print(step1Line4);

		setupState = allLowSetup;
		break;
	case allLowSetup:
		if(butState == proceed)
		{
			lcd.clear();
			lcd.home ();
			lcd.print(step2Line1);
			lcd.setCursor ( 0, 1 );
			lcd.print(step2Line2);
			lcd.setCursor ( 0, 2 );
			lcd.print(step2Line3);
			lcd.setCursor ( 0, 3 );
			lcd.print(step2Line4);

			setupState = allHighSetup;

			ReadAnalogSensors();

			settings.lowFrontL = frontLRaw;
			settings.lowFrontR = frontRRaw;
			settings.lowRearL = rearLRaw;
			settings.lowRearR = rearRRaw;
		}
		break;
	case allHighSetup:
		if(butState == proceed)
		{
			ReadAnalogSensors();

			settings.highFrontL = frontLRaw;
			settings.highFrontR = frontRRaw;
			settings.highRearL = rearLRaw;
			settings.highRearR = rearRRaw;

			int varienceFL = settings.highFrontL - settings.lowFrontL;
			///Serial.print("varienceFL ");
			///Serial.print(varienceFL);
			//Serial.println();
			int varienceFR = settings.highFrontR - settings.lowFrontR;
			//Serial.print("varienceFR ");
			//Serial.print(varienceFR);
			//Serial.println();
			int varienceRL = settings.highRearL - settings.lowRearL;
			//Serial.print("varienceRL ");
			//Serial.print(varienceRL);
			//Serial.println();
			int varienceRR = settings.highRearR - settings.lowRearR;
			//Serial.print("varienceRR ");
			//Serial.print(varienceRR);
			//Serial.println();

			if((abs(varienceFL) < MinSensorVarience) || (abs(varienceFR) < MinSensorVarience) || (abs(varienceRL) < MinSensorVarience) || (abs(varienceRR) < MinSensorVarience))
			{
				Serial.println("Out of varience");
				/*
				lcd.clear();
				lcd.home ();
				lcd.print(errorSetupLine1);
				lcd.setCursor ( 0, 1 );
				lcd.print(errorSetupLine2);
				lcd.setCursor ( 0, 2 );
				lcd.print(errorSetupLine3);
				lcd.setCursor ( 0, 3 );
				lcd.print(errorSetupLine4);
				setupState = errorSetup;
				*/
				//Make sure we load the last good settings 
				loadSettings();
				delay(SaveLoadSettingsDelay);

				ARDstate = dsError;
				errState = errSetup;
				setupState = unKnownSetup;
				break;
			}
			
			//Mark what sensors are installed upside down
			if(varienceFL < 0)
				settings.frontLFlag = varienceFL < 0 ? true : false;
			if(varienceFL < 0)
				settings.frontRFlag = varienceFR < 0 ? true : false;
			if(varienceFL < 0)
				settings.rearLFlag = varienceRL < 0 ? true : false;
			if(varienceFL < 0)
				settings.rearRFlag = varienceRR < 0 ? true : false;

			lcd.clear();
			lcd.home ();
			lcd.print(step3Line1);
			lcd.setCursor ( 0, 1 );
			lcd.print(step3Line2);
			lcd.setCursor ( 0, 2 );
			lcd.print(step3Line3);
			lcd.setCursor ( 0, 3 );
			lcd.print(step3Line4);

			setupState = finishedSetup;
		}
		break;
	case finishedSetup:
		if(butState == proceed)
		{
			ARDstate = dsNormal;
			setupState = unKnownSetup;
			lcd.clear();
			settings.isSetup = true;
			saveSettings();
			delay(SaveLoadSettingsDelay);
		}
		break;
	case errorSetup:
		/*
		//Make sure we load the last good settings 
		loadSettings();
		delay(SaveLoadSettingsDelay);

		if(butState == setupProceed)
		{
			ARDstate = dsNormal;
			setupState = unKnownSetup;
			lcd.clear();
		}
		break;
		*/
	default:
		break;
	}

}

//NormalMode()
void NormalMode()
{
	//Get raw sensor data
	ReadAnalogSensors();

	lcd.home ();
	
	/////////
	//Line 1
	/////////
	String temp;//(frontLDisplay);
	String displayString;//(normalLine1);

	//Front Left
	temp = String(frontLDisplay);
	displayString = String(normalLine1);
	
	if(temp.length() == 3)
	{
		displayString.setCharAt(0, temp.charAt(0));
		displayString.setCharAt(1, temp.charAt(1));
		displayString.setCharAt(2, temp.charAt(2));
	}
	else if(temp.length() == 2)
	{
		displayString.setCharAt(1, temp.charAt(0));
		displayString.setCharAt(2, temp.charAt(1));
	}
	else
		displayString.setCharAt(2, temp.charAt(0));

	//Front Right
	temp = String(frontRDisplay);
	if(temp.length() == 3)
	{
		displayString.setCharAt(17, temp.charAt(0));
		displayString.setCharAt(18, temp.charAt(1));
		displayString.setCharAt(19, temp.charAt(2));
	}
	else if(temp.length() == 2)
	{
		displayString.setCharAt(18, temp.charAt(0));
		displayString.setCharAt(19, temp.charAt(1));
	}
	else
		displayString.setCharAt(19, temp.charAt(0));

	lcd.print(displayString);
	//Serial.println(displayString);
	//Serial.println(frontLDisplay);
	
	/////////
	//Line 2
	/////////
	
	lcd.setCursor ( 0, 1 );
	lcd.print(normalLine2);

	/////////
	//Line 3
	/////////
	lcd.setCursor ( 0, 2 );

	if(tankPressureDisplay < 99)
	{
		lcd.print("         ");
		lcd.print(tankPressureDisplay);
	}
	else
	{
		lcd.print("        ");
		lcd.print(tankPressureDisplay);
	}

	/////////
	//Line 4
	/////////

	//Rear Left
	lcd.setCursor ( 0, 3 );
	temp = String(rearLDisplay);
	displayString = String(normalLine4);
	
	if(temp.length() == 3)
	{
		displayString.setCharAt(0, temp.charAt(0));
		displayString.setCharAt(1, temp.charAt(1));
		displayString.setCharAt(2, temp.charAt(2));
	}
	else if(temp.length() == 2)
	{
		displayString.setCharAt(1, temp.charAt(0));
		displayString.setCharAt(2, temp.charAt(1));
	}
	else
		displayString.setCharAt(2, temp.charAt(0));

	//Rear Right
	temp = String(rearRDisplay);
	if(temp.length() == 3)
	{
		displayString.setCharAt(17, temp.charAt(0));
		displayString.setCharAt(18, temp.charAt(1));
		displayString.setCharAt(19, temp.charAt(2));
	}
	else if(temp.length() == 2)
	{
		displayString.setCharAt(18, temp.charAt(0));
		displayString.setCharAt(19, temp.charAt(1));
	}
	else
		displayString.setCharAt(19, temp.charAt(0));

	lcd.print(displayString);
	

	if(butState == enterSetupMode)
	{
		ARDstate = dsSetup;
		setupState = enterSetup;
	}
}

//AnimationMode()
void AnimationMode()
{

}
void RideHeightMode()
{
	switch (rideHeightState)
	{
	case enterRideHeight:
		lcd.clear();
		lcd.home ();
		lcd.print(step1rideHeightSetLine1);
		lcd.setCursor ( 0, 1 );
		lcd.print(step1rideHeightSetLine2);
		lcd.setCursor ( 0, 2 );
		lcd.print(step1rideHeightSetLine3);
		lcd.setCursor ( 0, 3 );
		lcd.print(step1rideHeightSetLine4);

		setupState = setRideHeight;
		break;
	case setRideHeight:
		if(butState == proceed)
		{

		}
		break;
	case finishedRideHeight:
		break;
	default:
		break;
	}
}

//ErrorMode()
void ErrorMode()
{
	static bool isDisplayingError = false;
	
	if(!isDisplayingError)
	{
		switch (errState)
		{
		case errSetup:
			lcd.clear();
			lcd.home ();
			lcd.print(errorSetupLine1);
			lcd.setCursor ( 0, 1 );
			lcd.print(errorSetupLine2);
			lcd.setCursor ( 0, 2 );
			lcd.print(errorSetupLine3);
			lcd.setCursor ( 0, 3 );
			lcd.print(errorSetupLine4);

			isDisplayingError = true;
			break;
		case errEEPROM:
			lcd.clear();
			lcd.home ();
			lcd.print(errorEEPROMLine1);
			lcd.setCursor ( 0, 1 );
			lcd.print(errorEEPROMLine2);
			lcd.setCursor ( 0, 2 );
			lcd.print(errorEEPROMLine3);
			lcd.setCursor ( 0, 3 );
			lcd.print(errorEEPROMLine4);

			isDisplayingError = true;
			break;
		}
	}

	if(butState == proceed)
	{
		ARDstate = dsNormal;
		errState = errNone;
		isDisplayingError = false;
		lcd.clear();
	}
}

void ReadAnalogSensors()
{
#ifndef RAWVALUEMODE

	int smoothedFrontRRaw = frontRRaw;
	int smoothedFrontLRaw = frontLRaw;
	int smoothedRearRRaw = rearRRaw;
	int smoothedRearLRaw = rearLRaw;

	for (int i = 0; i< FILTERSAMPLERATE; i++)
	{
		frontRRaw = analogRead(HeightFR_pin);
		smoothedFrontRRaw =  smooth(frontRRaw, SMOOTHFILTER, smoothedFrontRRaw);
		delay(SensorReadDelay); 

		frontLRaw = analogRead(HeightFL_pin);
		smoothedFrontLRaw =  smooth(frontLRaw, SMOOTHFILTER, smoothedFrontLRaw);
		delay(SensorReadDelay); 

		rearRRaw = analogRead(HeightRR_pin);
		smoothedRearRRaw =  smooth(rearRRaw, SMOOTHFILTER, smoothedRearRRaw);
		delay(SensorReadDelay); 

		rearLRaw = analogRead(HeightRL_pin);
		smoothedRearLRaw =  smooth(rearLRaw, SMOOTHFILTER, smoothedRearLRaw);
		delay(SensorReadDelay); 
    }

	//Set raw to smoothed
	frontRRaw = smoothedFrontRRaw;
	frontLRaw = smoothedFrontLRaw;
	rearRRaw = smoothedRearRRaw;
	rearLRaw = smoothedRearLRaw;

#else

	frontRRaw = analogRead(HeightFR_pin);
	delay(SensorReadDelay);
	frontLRaw = analogRead(HeightFL_pin);
	delay(SensorReadDelay);
	rearRRaw  = analogRead(HeightRR_pin);
	delay(SensorReadDelay);
	rearLRaw  = analogRead(HeightRL_pin);
	delay(SensorReadDelay);

#endif
	
	tankPressureRaw = analogRead(PressureTank_pin);
	delay(SensorReadDelay);

	////////////////////////////////////////////////////////////
	//Convert raw ride height sensor values to  0-DISPLAY_MAX
	////////////////////////////////////////////////////////////

#ifndef RAWVALUEMODE
	int tempDispVal = 0;
	//Front Right 
	tempDispVal   = (int)(((frontRRaw-settings.lowFrontR)/(settings.highFrontR-settings.lowFrontR))*DISPLAY_MAX);
	frontRDisplay = tempDispVal;//settings.frontRFlag == true ? DISPLAY_MAX - tempDispVal : tempDispVal;
	//Serial.println(frontRRaw);
	//Serial.println(settings.lowFrontR);
	//Serial.println(settings.highFrontR);
	//Serial.println(DISPLAY_MAX);

	//Front Left
	tempDispVal   = (int)(((frontLRaw-settings.lowFrontL)/(settings.highFrontL-settings.lowFrontL))*DISPLAY_MAX);
	frontLDisplay = tempDispVal;//settings.frontLFlag == true ? DISPLAY_MAX - tempDispVal : tempDispVal;

	//Rear Right
	tempDispVal  = (int)(((rearRRaw-settings.lowRearR)/(settings.highRearR-settings.lowRearR))*DISPLAY_MAX);
	rearRDisplay = tempDispVal;//settings.rearRFlag == true ? DISPLAY_MAX - tempDispVal : tempDispVal;

	//Rear Left
	tempDispVal  = (int)(((rearLRaw-settings.lowRearL)/(settings.highRearL-settings.lowRearL))*DISPLAY_MAX);
	rearLDisplay = tempDispVal;//settings.rearLFlag == true ? DISPLAY_MAX - tempDispVal : tempDispVal;

	//Convert raw tank pressure value to actual psi 0-200psi
	tankPressureDisplay = (int)(((tankPressureRaw-PSI_MIN_VALUE)/(PSI_MAX_VALUE-PSI_MIN_VALUE))*PSI_MAX);
#else
	frontRDisplay = frontRRaw;
	frontLDisplay = frontLRaw;
	rearRDisplay = rearRRaw;
	rearLDisplay = rearLRaw;
	tankPressureDisplay = tankPressureRaw;
#endif
}

int getInputState()
{
	///////////////////////////////////////////////
	//#define Button01_pin 42 //Lower Left (Red)
	//#define Button02_pin 44 //Upper Left (Black)
	//#define Button03_pin 46 //Lower Right (Red)
	//#define Button04_pin 48 //Upper Right (Black)
	////////////////////////////////////////////////
	 
	//Toggle Backlight
	if ( button01.uniquePress())
	{
		return toggleBacklight;
	}
	
	//Enter setup mode
	if(button02.heldFor(100) && button04.uniquePress())
	{
		return enterSetupMode;
	}

	//Enter ride height mode
	if(button02.heldFor(100) && button03.uniquePress())
	{
		return enterRideHeightMode;
	}
	//Proceed
	if(button03.uniquePress())
	{
		return proceed;
	}

	//
	//if(button04.isPressed())
	//{
	//	lcd.print("button04");
	//}
	 
	return none;
}

int smooth(int data, float filterVal, float smoothedVal){


  if (filterVal > 1){      // check to make sure param's are within range
    filterVal = .99;
  }
  else if (filterVal <= 0){
    filterVal = 0;
  }

  smoothedVal = (data * (1 - filterVal)) + (smoothedVal  *  filterVal);

  return (int)smoothedVal;
}

//Load settings from EEPROM
void loadSettings()
{
#ifdef DEBUG
	/*
	uint8_t temp = EEPROM.read(CONFIG_START + sizeof(settings) - 2);
	Serial.print("Verify : ");
	Serial.println(temp);

	temp = EEPROM.read(CONFIG_START + sizeof(settings) - 3);
	Serial.print("Verify : ");
	Serial.println(temp);

	temp = EEPROM.read(CONFIG_START + sizeof(settings) - 4);
	Serial.print("Verify : ");
	Serial.println(temp);

	temp = (uint8_t)settings.version_of_program[2];
	Serial.print("Verify : ");
	Serial.println(temp);

	temp = (uint8_t)settings.version_of_program[1];
	Serial.print("Verify : ");
	Serial.println(temp);

	temp = (uint8_t)settings.version_of_program[0];
	Serial.print("Verify : ");
	Serial.println(temp);
	*/
#endif
	// To make sure there are settings, and they are YOURS!
	// If nothing is found it will use the default settings.
	if (EEPROM.read(CONFIG_START + sizeof(settings) - 2) == (uint8_t)settings.version_of_program[2] &&
		EEPROM.read(CONFIG_START + sizeof(settings) - 3) == (uint8_t)settings.version_of_program[1] &&
		EEPROM.read(CONFIG_START + sizeof(settings) - 4) == (uint8_t)settings.version_of_program[0])
	{ // reads settings from EEPROM
		Serial.println("LoadSettings");
		eeprom_read_block((void*)&settings, (void*)CONFIG_START, sizeof(settings));
		printSettings();
	}
	else
	{
		// settings aren't valid! will overwrite with default settings
		saveSettings();
	}
}

//Save settings to EEPROM
void saveSettings()
{
	Serial.println("SaveSettings");
	printSettings();
	//Write our settings to the EEPROM
	eeprom_write_block((const void*)&settings, (void*)CONFIG_START, sizeof(settings));
	//printSettings();
	// Verify settings
	//char versionCheck[4];
	//eeprom_read_block((void*)versionCheck, (void*)(CONFIG_START+sizeof(settings)-sizeof(SETTINGS_VERSION)), sizeof(SETTINGS_VERSION));
	//for (unsigned int t=0; t<sizeof(settings)+10; t++)
	//{
		// writes to EEPROM
		//EEPROM.write(CONFIG_START + t, 0);//*((char*)&settings + t));
		// and verifies the data
		//if (EEPROM.read(CONFIG_START + t) != *((char*)&settings + t))
		//{
		  // error writing to EEPROM
			//Serial.println(t);
		//}
	//}
	/*
	for(int t=0; t<
	if(versionCheck[t] != settings_version[t])
	{
		// error writing to EEPROM
		ARDstate = dsError;
		errState = errEEPROM;
#ifdef DEBUG
		Serial.println("Error: errEEPROM");
		Serial.print("Expecting: ");
		Serial.println(SETTINGS_VERSION);
		Serial.print("Received: ");
		Serial.println(versionCheck);
#endif
	}
	*/

#ifdef DEBUG
		//delay(100);
		//eeprom_read_block((void*)&settings, (void*)CONFIG_START, sizeof(settings));
		//delay(100);

		//printSettings();
		//Serial.print("Verified  ");
		//Serial.println(settings.version_of_program);
#endif
}

#ifdef DEBUG
void printSettings()
{
	Serial.println("Settings:  ");
	Serial.print("frontLFlag  ");
	Serial.println(settings.frontLFlag);
	Serial.print("frontRFlag  ");
	Serial.println(settings.frontRFlag);
	Serial.print("rearLFlag  ");
	Serial.println(settings.rearLFlag);
	Serial.print("rearRFlag  ");
	Serial.println(settings.rearRFlag);
	Serial.print("highFrontL  ");
	Serial.println(settings.highFrontL);
	Serial.print("highFrontR  ");
	Serial.println(settings.highFrontR);
	Serial.print("highRearL  ");
	Serial.println(settings.highRearL);
	Serial.print("highRearR  ");
	Serial.println(settings.highRearR);
	Serial.print("isSetup  ");
	Serial.println(settings.isSetup);
	Serial.print("lcdBacklightON  ");
	Serial.println(settings.lcdBacklightON);
	Serial.print("lowFrontL  ");
	Serial.println(settings.lowFrontL);
	Serial.print("lowFrontR  ");
	Serial.println(settings.lowFrontR);
	Serial.print("lowRearL  ");
	Serial.println(settings.lowRearL);
	Serial.print("lowRearR  ");
	Serial.println(settings.lowRearR);
	Serial.print("tankPressure  ");
	Serial.println(settings.tankPressure);
	Serial.print("version_of_program  ");
	Serial.println(settings.version_of_program);
	Serial.println();
}

void erraseEEPROMZeros()
{
	for (unsigned int t=0; t<sizeof(settings)+10; t++)
	{
		EEPROM.write(CONFIG_START + t, 0);
	}

}

#endif