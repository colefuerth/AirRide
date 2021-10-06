/*
    <DisplayStrings.h>
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
 * DisplayStrings.h			       //
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

#ifndef DISPLAY_STRINGS_H
#define DISPLAY_STRINGS_H

//Configure screen step one
const String step1Line1 = "Release all the air ";
const String step1Line2 = "from air bags then  ";
const String step1Line3 = "proceed.            ";
const String step1Line4 = "          Proceed-->";

const String step2Line1 = "Inflate air bags    ";
const String step2Line2 = "to maximum height   ";
const String step2Line3 = "then proceed.       ";
const String step2Line4 = "          Proceed-->";

const String step3Line1 = "Congratulations!!   ";
const String step3Line2 = "Configuration has   ";
const String step3Line3 = "been completed.     ";
const String step3Line4 = "          Proceed-->";

//const String normalLine1 = "00    F R O N T   00";
const String normalLine1 = "000 F/L      F/R 000";
//const String normalLine2 = "        PSI         ";
const String normalLine2 = "        TANK        ";
const String normalLine3 = "         0          ";
//const String normalLine4 = "00    R E A R     00";
const String normalLine4 = "000 R/L      R/R 000"; 

const String errorSetupLine1 = "The sensors are not ";
const String errorSetupLine2 = "installed correctly!";
const String errorSetupLine3 = "More sensor range is";
const String errorSetupLine4 = "needed.  Proceed -->";

const String errorEEPROMLine1 = "The EEPROM failed to";
const String errorEEPROMLine2 = "write to memory!    ";
const String errorEEPROMLine3 = "                    ";
const String errorEEPROMLine4 = "          Proceed-->";

const String step1rideHeightSetLine1 = "To set ride height. ";
const String step1rideHeightSetLine2 = "Make sure vehicle   ";
const String step1rideHeightSetLine3 = "is on level surface.";
const String step1rideHeightSetLine4 = "          Proceed-->";

const String step2rideHeightSetLine1 = "000 F/L      F/R 000";
const String step2rideHeightSetLine2 = "  Set ride height   ";
const String step2rideHeightSetLine3 = "000 R/L      R/R 000";
const String step2rideHeightSetLine4 = "          Proceed-->";

const String rideHeightIsSetLine1 = "Ride height has been";
const String rideHeightIsSetLine2 = "set!                ";
const String rideHeightIsSetLine3 = "                    ";
const String rideHeightIsSetLine4 = "          Proceed-->";

const char settings_version[4] = SETTINGS_VERSION;

//forward slash? 0x0,0x10,0x8,0x4,0x2,0x1,0x0,0x0
//const String truckArtLine1 = "........_____ ......";
//const String truckArtLine2a = " ______//";//__{\______.";
//const String truckArtLine2b = " //__{";
//const String truckArtLine2c = " \";
//const String truckArtLine2d = "______.";
//const String truckArtLine3 = "/__(O)__//___/_(O)_/";
//const String truckArtLine4 = "....................";
#endif