/*
    <AirRideDisplay.h Reads and distributes data to an lcd>
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
 * AirRideDisplay.h			       //
 *								   //
 * Author: Joseph E Cannon		   //
 * Created: 10/3/2013			   //
 *								   //
...................................//
..............._____ ..............//
........______//__{\______ ........//
....../__(O)__//___/__(O)_/........//
...................................//
*////////////////////////////////////

#ifndef AIRRIDEDISPLAY_H_
#define AIRRIDEDISPLAY_H_

#include <I2CIO.h>
#include <FastIO.h>
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <button.h>

#define DEBUG

//#define RAWVALUEMODE
/////////////////////////////////////////////////////
//Save data
/////////////////////////////////////////////////////

// ID of the settings block
#define SETTINGS_VERSION "qlp"

// Tell it where to store your config data in EEPROM
#define CONFIG_START 32

///////////////////////////////////////////////////////////////
// Ride height and tank pressure sensor pins (analog input, +5v)
// Delphi ride height sensors (.5v-4.5v)
// Pressure sensor 0.5V - 4.5V linear voltage output.
// 0 psi outputs 0.5V, 100 psi outputs 2.5V, 200 psi outputs 4.5V.
///////////////////////////////////////////////////////////////

#define ANALOG_MAX 1024.00
#define ANALOG_MIN 0.00

#define DISPLAY_MAX 400.00
#define DISPLAY_MIN 0.00

#define PSI_MAX 300.00 
#define PSI_MIN 0.00

//on @ 151-152(520) off@ 182(610) 0psi@74-75 display 8-9
//165on 200off
#define PSI_MAX_VALUE 870.00//(4.5/5)*ANALOG_MAX
#define PSI_MIN_VALUE 75.00//(0.5/5)*ANALOG_MAX

//Sensor smoothing filter value 
//0 is off,  .9999 is max smooth 
#define SMOOTHFILTER .0500

//Number of reads for filtering
#define FILTERSAMPLERATE 10

//Tank
#define PressureTank_pin  0

//Front Right
//#define PressureFR_pin  0
#define HeightFR_pin  1

//Front Left
//#define PressureFL_pin  1
#define HeightFL_pin  2

//Rear Right
//#define PressureRR_pin  2
#define HeightRR_pin  3

//Rear Left
//#define PressureRL_pin  3
#define HeightRL_pin  4

//This value represents the mininum sensor sweep range 
//allowed for proper sensor resolution.
#define MinSensorVarience 400

//Amount of delay between sensor read calls
#define SensorReadDelay 10

//Amount of delay between saveSettings and loadSettings calls
#define SaveLoadSettingsDelay 100

//Amount that the ride height may be off by and still be in proper range
#define RideHeightVarience 10
//Settings
struct StoreStruct
{
	//Have we ran the setup sequence yet
	bool isSetup;
	//Sensor values
	float lowFrontR, highFrontR;
	float lowFrontL, highFrontL;
	float lowRearR, highRearR;
	float lowRearL, highRearL;
	//Flags to identify if the sensor is mounted upside down
	bool frontRFlag, frontLFlag;
	bool rearRFlag, rearLFlag;
	//LCD backlight
	bool lcdBacklightON;
	//Tank pressure
	int tankPressure;
	//Current ride height values
	int rideHeightFrontR, rideHeightFrontL;
	int rideHeightRearR, rideHeightRearL;
	// This is for mere detection if they are your settings
	char version_of_program[4];
	} settings = {
	// The default values
	false,
	ANALOG_MIN, ANALOG_MAX,
	ANALOG_MIN, ANALOG_MAX,
	ANALOG_MIN, ANALOG_MAX,
	ANALOG_MIN, ANALOG_MAX,
	false, true,
	false, true,
	
	true,
	0,

	500, 500,
	500, 500,
	SETTINGS_VERSION
};

/////////////////////////////////////////////////////
//LCD
/////////////////////////////////////////////////////

//LCD I2C
#define I2C_ADDR    0x3F
#define BACKLIGHT_PIN     3
#define En_pin  2
#define Rw_pin  1
#define Rs_pin  0
#define D4_pin  4
#define D5_pin  5
#define D6_pin  6
#define D7_pin  7

//Setup LCD display object
LiquidCrystal_I2C lcd(I2C_ADDR, En_pin, Rw_pin, Rs_pin, D4_pin, 
					  D5_pin, D6_pin, D7_pin, BACKLIGHT_PIN, POSITIVE);

///////////////////////////////////////////////////////////////
//Display panel button pins (Digital input, internal pullup)
///////////////////////////////////////////////////////////////

#define Button01_pin 42 //Lower Left (Red)
#define Button02_pin 44 //Upper Left (Black)
#define Button03_pin 46 //Lower Right (Red)
#define Button04_pin 48 //Upper Right (Black)

//Create button objects
Button button01 = Button( Button01_pin, BUTTON_PULLUP_INTERNAL );
Button button02 = Button( Button02_pin, BUTTON_PULLUP_INTERNAL );
Button button03 = Button( Button03_pin, BUTTON_PULLUP_INTERNAL );
Button button04 = Button( Button04_pin, BUTTON_PULLUP_INTERNAL );

///////////////////////////////////////////////////////////////
//Function declarations
///////////////////////////////////////////////////////////////

void SetupMode();
void NormalMode();
void AnimationMode();
void RideHeightMode();
void ErrorMode();
void ReadAnalogSensors();
void loadSettings();
void saveSettings();
int getInputState();

//Raw input values from ride height and tank pressure sensors
int frontRRaw = 0;
int frontLRaw = 0;
int rearRRaw = 0;
int rearLRaw = 0;

int tankPressureRaw = 0;

int frontRDisplay = 0;
int frontLDisplay = 0;
int rearRDisplay = 0;
int rearLDisplay = 0;

int tankPressureDisplay = 0;

///////////////////////////////////////////
//States
///////////////////////////////////////////
enum ARDStates
{
	dsSetup,
	dsNormal,
	dsAnimation,
	dsSetRideHeight,
	dsError
};
int ARDstate = dsNormal;

//Setup states
enum setupStates
{
	enterSetup,
	allLowSetup,
	allHighSetup,
	finishedSetup,
	errorSetup,
	unKnownSetup
};
int setupState = unKnownSetup;

//Set ride height state
enum setRideHieghtstate
{
	enterRideHeight,
	setRideHeight,
	finishedRideHeight,
	unKnownRideHeight
};
int rideHeightState = unKnownRideHeight;

//Button states
enum buttonStates
{
	enterSetupMode,
	exitSetupMode,
	toggleBacklight,
	proceed,
	rideHeightSet,
	enterRideHeightMode,
	none
};
int butState = none;

//Button states
enum errorModeState
{
	errSetup,
	errEEPROM,
	errNone
};
int errState = none;
#endif /* AIRRIDEDISPLAY_H_ */