# 1 "/home/pi/Air Ride Project I/Arduino Project/AirRide.cpp"
# 2 "/home/pi/Air Ride Project I/Arduino Project/AirRide.cpp" 2
# 3 "/home/pi/Air Ride Project I/Arduino Project/AirRide.cpp" 2
# 4 "/home/pi/Air Ride Project I/Arduino Project/AirRide.cpp" 2
# 5 "/home/pi/Air Ride Project I/Arduino Project/AirRide.cpp" 2

// ------------------- SHOCK OBJECT FUNCTIONS -----------------------

// initialize a Shock, with all pin IO values
Shock::Shock(uint8_t valve_pin_out, uint8_t height_pin, uint8_t pres_pin)
{
    this->_valve.setPin(valve_pin_out);
    this->_pressure.setPin(pres_pin);
    this->_height.setPin(height_pin);
}

// set target height Shock should try to maintain
// also sets Shock to maintain a certain height
void Shock::setHeight(float h_mm)
{
    this->_mm_target = h_mm;
    this->_mode = 2;
}

// set target pressure that Shock should maintain
// sets Shock control mode to maintain psi rather than height
void Shock::setPressure(float psi)
{
    this->_psi_target = psi;
    this->_mode = 1;
}

// check height on sensor associated with Shock
float Shock::getHeight()
{
    return this->_height.h_mm();
}

// check pressure in psi on sensor associated with Shock
float Shock::getPressure()
{
    return this->_pressure.psi();
}

void Shock::update()
{
    // TODO: use _mode and height/psi values to maintain a target setting
}

// ------------------- COMPRESSOR OBJECT FUNCTIONS -----------------------

// pass a Motor pin and pressure sensor analog pin.
Compressor::Compressor(uint8_t motor_control_pin, uint8_t pres_sensor_pin)
{
    this->_pressure.setPin(pres_sensor_pin);
    this->_mtr.setPin(motor_control_pin);
}

// pressure (PSI) for Compressor to maintain when active
void Compressor::setPressure(float psi)
{
    this->_runPres = psi;
}

// pressure (PSI) for Compressor to maintain when idle
void Compressor::setIdlePressure(float psi)
{
    this->_idlePres = psi;
}

// return pressure in PSI
float Compressor::getPressure()
{
    return this->_pressure.psi();
}

// fetch Motor state
bool Compressor::isFilling()
{
    return this->_mtr.getState();
}

// 0 if off, 1 if active, 2 if idle
int Compressor::getState()
{
    if (!this->_active) // inactive
        return 0;
    else if (!this->_idle) // active and not idle
        return 1;
    else // active and idle
        return 2;
}

// store EEPROM state of Compressor
void Compressor::eeprom_store(uint16_t *addr)
{
    // start by checking initializer
    if (EEPROM.read(*addr) != 0xFE)
    {
        debug_log("Compressor EEPROM uninitialized at location " + String(*addr) + ", initializing.");
        EEPROM.write(*addr, 0xFE); // initializer
    }
    *addr++; // move pointer past initializer

    // bulk of data stored
    // TODO: ADD DATA TO BE STORED BETWEEN POWER CYCLES
    this->_pressure.eeprom_store(addr);
    // TODO: compressor needs a bleed valve
    // this->_valve.eeprom_store(addr);

    /*

    // spare integers

    ee_write_int(addr, 0);

    ee_write_int(addr, 0);

    ee_write_int(addr, 0);

    ee_write_int(addr, 0);

    */
# 118 "/home/pi/Air Ride Project I/Arduino Project/AirRide.cpp"
    EEPROM.update(*addr++, 0xFF); // close with FF byte
}

// start regulating tank pressure at running pressure
void Compressor::start()
{
    _active = true;
    _idle = false;
}

// set Compressor to idle mode, maintaining stored idle PSI
void Compressor::start_idle()
{
    _active = true;
    _idle = true;
}

// stop regulating tank pressure
void Compressor::stop()
{
    _active = false;
}

// main update loop
void Compressor::update()
{
    // set target PSI based on Compressor state
    if (_idle)
        _tgtPres = _idlePres;
    else
        _tgtPres = _runPres;

    // if tank is active, monitor pressure and run Motor to maintain target
    if (_active)
    {
        // start Motor case, requirements: pres too low, not in cooldown, Motor is off
        if (_mtr.getState() == 0 && _pressure.psi() < (_tgtPres - _hysteresis) && !_cooldown)
        {
            _mtr.setState(1);
            _mtrLastStateChangeTime = millis();
        }
        // stop Motor case, requirements: Motor is on and pres at spec or max time reached
        if (_mtr.getState() == 1)
        {
            if (_pressure.psi() >= (_tgtPres + _hysteresis) || _pressure.psi() >= 150 /* absolute maximum pressure before ESTOP and fault*/)
            {
                _mtr.setState(0);
                _mtrLastStateChangeTime = millis();
            }
            if (millis() - _mtrLastStateChangeTime >= (long)1 * 60 /* 1 minute cooldown before running again*/ * 1000)
            {
                _mtr.setState(0);
                _mtrLastStateChangeTime = millis();
                _cooldown = true;
            }
        }
    }
    // if inactive, then ensure Motor is off
    else
    {
        if (_mtr.getState() == 1)
            _mtr.setState(0);
        _mtrLastStateChangeTime = millis();
    }

    // if cooldown was engaged, then disengage it when the timer runs out.
    // Cooldown must finish once started before Motor starts again, regardless of machine state.
    if (_cooldown && millis() - _mtrLastStateChangeTime >= (long)1 * 60 /* 1 minute cooldown before running again*/ * 1000)
        _cooldown = false;
}

// ******** EEPROM PROFILE FUNCTIONS **********

// looks up EEPROM address of profile
// returns 0 if not found, or the address of the profile in EEPROM if exists
uint16_t profileExists(int _profile_index)
{
    // start by pointing at integer value for number of profiles initialized
    uint16_t addr = 500 /* profiles starting address*/ + 3;
    // check for uninitialized EEPROM or not enough profiles
    if (ee_initialized() || ee_read_int(&addr) - 1 < _profile_index)
        return 0;
    // move pointer to relevant pointer
    // at this point, address pointer is pointing at first address of first profile.
    // TODO: instead of 28 size, have a define table for each eeprom item size
    addr += _profile_index * 28; // each profile is 28 bytes wide
    // return address pointed to
    return addr;
}

// creates a new profile, and fills the profile with default values.
// 0: profile creation success
// 1: max profiles reached
// 2: EEPROM uninitialized
int createProfile(Profile *_new_profile)
{
    if (0)
        debug_log("start of createprofile call");
    if (ee_initialized()) // check if eeprom is initialized
    {
        debug_log("Unable to create new profile; a problem was found when checking EEPROM validity.");
        return 2;
    }
    uint16_t addr = 500 /* profiles starting address*/ + 3; // set address to location of profile counter
    int numprofiles = ee_read_int(&addr);
    if (numprofiles >= 5) // check if too many profiles exist
    {
        debug_log("Unable to create new profile, maximum number of profiles reached!");
        return 1;
    }
    addr -= 2; // reset address pointer to profile counter in EEPROM
    ee_write_int(&addr, numprofiles + 1); // update profile counter in eeprom
    addr += numprofiles * 26; // increment to the starting address of new profile
    if (0)
        debug_log("creating profile " + String(numprofiles) + " at address " + String(addr));
    EEPROM.update(addr++, 0xFE); // initializer
    ee_write_string(&addr, _new_profile->name); // store profile name
    ee_write_int(&addr, _new_profile->mode); // store mode
    ee_write_int(&addr, _new_profile->val); // store value
    EEPROM.update(addr++, 0xFF);
    if (0)
        debug_log("done creating profile.");

    return 0; // success condition
}

// loads profile indexed by '_index'.
// index ranges from 0-4
// 0: successful load
// 1: Master EEPROM problem
// 2: Profile not yet created
// 3: Profile data corrupted
int loadProfile(Profile *_profile, int _index)
{
    if (0)
        debug_log("loading profile " + String(_index));
    // check if eeprom is initialized
    if (int err = ee_initialized())
    {
        debug_log("EEPROM error " + String(err));
        debug_log("Unable to load profile; a problem was found when checking EEPROM validity.");
        return 1;
    }
    // address pointer is 0 if not exist
    uint16_t addr = profileExists(_index);
    if (!addr || _index > 4)
    {
        debug_log("Unable to load profile" + String(_index) + "; specified profile does not exist");
        return 2; // unsuccessful condition
    }
    if (EEPROM.read(addr++) != 0xFE)
    {
        debug_log("Profile data corrupted. Index: " + String(_index));
        return 3;
    }

    // load body data, this method is verified
    _profile->name = ee_read_string(&addr);
    _profile->mode = ee_read_int(&addr);
    _profile->val = ee_read_int(&addr);

    // check for correct termination
    // NOTE: this is a WARNING, and will still load the data regardless
    if (EEPROM.read(addr++) != 0xFF)
    {
        debug_log("Warning: Profile data corrupted; improper termination. Index: " + String(_index));
        return 3;
    }
    if (0)
        debug_log("done loading profile " + String(_index));
    return 0; // success condition
}

// im pretty sure this stores strings improperly
// saves profile _profile, in profile index _index.
// index ranges from 0-4
// 0: successful read
// 1: EEPROM problem
// 2: Profile slot uninitialized
// 3: Profile data corrupted
int saveProfile(Profile *_profile, int _index)
{
    if (0)
        debug_log("saving profile " + String(_index));
    // check if eeprom is initialized
    if (ee_initialized())
    {
        debug_log("Unable to save profile; a problem was found when checking EEPROM validity.");
        return 1;
    }
    // address pointer is 0 if not exist
    uint16_t addr = 500 /* profiles starting address*/ + 3; // point address at number of profiles initialized
    // index is 0-4, number initialized is 1-5
    // so, if 1 profile is initialized but index is 1, then fail
    if (_index >= ee_read_int(&addr) || _index > 4)
    {
        debug_log("Unable to save profile; specified profile does not exist");
        return 2; // unsuccessful condition
    }

    // increment address pointer to desired start of profile
    int profile_size = 22 + (2 * sizeof(int)); // 20 for string, 2 for init and termination, and 2 ints
    addr += _index * profile_size; // move to initializer

    // all profiles begin with FE when initialized
    if (EEPROM.read(addr++) != 0xFE)
    {
        debug_log("Profile data corrupted. Index: " + String(_index));
        return 3;
    }

    // load body data, this method is verified
    ee_write_string(&addr, _profile->name);
    ee_write_int(&addr, _profile->mode);
    ee_write_int(&addr, _profile->val);

    // check for correct termination
    // NOTE: this is a WARNING, and will still load the data regardless
    if (EEPROM.read(addr++) != 0xFF)
    {
        debug_log("Warning: Profile data corrupted; improper termination. Index: " + String(_index));
        return 3;
    }
    if (0)
        debug_log("done saving profile " + String(_index));
    return 0; // success condition
}
# 1 "/home/pi/Air Ride Project I/Arduino Project/main.ino"
// arduino usb port at 'dmesg | grep "tty"'

# 4 "/home/pi/Air Ride Project I/Arduino Project/main.ino" 2
# 5 "/home/pi/Air Ride Project I/Arduino Project/main.ino" 2
# 6 "/home/pi/Air Ride Project I/Arduino Project/main.ino" 2
# 7 "/home/pi/Air Ride Project I/Arduino Project/main.ino" 2
# 8 "/home/pi/Air Ride Project I/Arduino Project/main.ino" 2



# 10 "/home/pi/Air Ride Project I/Arduino Project/main.ino"
Shock *shocks[4];
Compressor *cmp;

// I use this a lot so its easier to define it
// used for storing constant strings in flash instead of ram
// using char array instead of strings since it saves more space


// any function definitions used in main
// cannot use cli(), that is an interrupts function
void aircli();

void setup()
{
 // use defined BAUD_R
 Serial.begin(115200); // enable serial for debug
 while (!Serial)
  delay(10);

 // TODO: integrate i2c boards to get shock inSerial.println
 for (int i = 0; i < 4; i++)
  shocks[i] = new Shock(2 + i, 2 + 4 + i, 2 + 8 + i);
 cmp = new Compressor(2 + 12, 2 + 12 + 1);
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
  const uint8_t bufstep = 32; // step buffer sizes 32 bytes at a time
  static char *buf = 
# 66 "/home/pi/Air Ride Project I/Arduino Project/main.ino" 3 4
                    __null
# 66 "/home/pi/Air Ride Project I/Arduino Project/main.ino"
                        ; // initialized as null
  // TODO: make sure serial messages from pi end with \n,
  // to accurately separate commands when sent in fast succession
  int i = 0;
  while (Serial.available() > 0) // process response
  {
   if (bufsize >= 256)
   {
    const static char str[] 
# 74 "/home/pi/Air Ride Project I/Arduino Project/main.ino" 3
   __attribute__((__progmem__)) 
# 74 "/home/pi/Air Ride Project I/Arduino Project/main.ino"
            = "cli buffer overflow!";
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
  static char **argv = 
# 94 "/home/pi/Air Ride Project I/Arduino Project/main.ino" 3 4
                      __null
# 94 "/home/pi/Air Ride Project I/Arduino Project/main.ino"
                          ;
  if (argc == 0) // first check, set up arguement handler
   argv = (char **)malloc((argc = 1) * sizeof(char *));

  argv[0] = strtok(buf, " "); // set first string
  char *inpstr;
  int x = 1;
  while (inpstr = strtok(
# 101 "/home/pi/Air Ride Project I/Arduino Project/main.ino" 3 4
                        __null
# 101 "/home/pi/Air Ride Project I/Arduino Project/main.ino"
                            , " ")) // split string into separate args
  {
   x++;
   if (x > argc) // if need more space for another arg
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
      const static char str[] 
# 253 "/home/pi/Air Ride Project I/Arduino Project/main.ino" 3
     __attribute__((__progmem__)) 
# 253 "/home/pi/Air Ride Project I/Arduino Project/main.ino"
              = "FAULT: unknown compressor mode";
      Serial.println(str);
     }
    }
    else
    {
     const static char str[] 
# 259 "/home/pi/Air Ride Project I/Arduino Project/main.ino" 3
    __attribute__((__progmem__)) 
# 259 "/home/pi/Air Ride Project I/Arduino Project/main.ino"
             = "Unrecognized shock 'get' arguement";
     debug_log(str);
    }
   }
   // start: start compressor
   else if (!strcmp(argv[1], "start"))
   {
    cmp->start();
    const static char str[] 
# 267 "/home/pi/Air Ride Project I/Arduino Project/main.ino" 3
   __attribute__((__progmem__)) 
# 267 "/home/pi/Air Ride Project I/Arduino Project/main.ino"
            = "Compressor started in run mode, regulating pressure at ";
    debug_log(str + String(cmp->_runPres) + "psi.");
   }
   // stop: stop compressor running
   else if (!strcmp(argv[1], "stop"))
   {
    cmp->stop();
    const static char str[] 
# 274 "/home/pi/Air Ride Project I/Arduino Project/main.ino" 3
   __attribute__((__progmem__)) 
# 274 "/home/pi/Air Ride Project I/Arduino Project/main.ino"
            = "Compressor stopped regulating pressure.";
    debug_log(str);
   }
   // idle: start compressor in idle mode
   else if (!strcmp(argv[1], "idle"))
   {
    cmp->start_idle();
    const static char str[] 
# 281 "/home/pi/Air Ride Project I/Arduino Project/main.ino" 3
   __attribute__((__progmem__)) 
# 281 "/home/pi/Air Ride Project I/Arduino Project/main.ino"
            = "Compressor started in idle mode, regulating pressure at ";
    debug_log(str + String(cmp->_idlePres) + "psi.");
   }
   // bleed: open bleed valve and drop pressure to 0
   else if (!strcmp(argv[1], "bleed"))
   {
    // TODO: compressor needs a bleed function
    // cmp->bleed();
    const static char str[] 
# 289 "/home/pi/Air Ride Project I/Arduino Project/main.ino" 3
   __attribute__((__progmem__)) 
# 289 "/home/pi/Air Ride Project I/Arduino Project/main.ino"
            = "Bleeding compressor";
    debug_log(str);
    debug_log("NO BLEED FUNCTION MADE YET");
   }
   else
   {
    const static char str[] 
# 295 "/home/pi/Air Ride Project I/Arduino Project/main.ino" 3
   __attribute__((__progmem__)) 
# 295 "/home/pi/Air Ride Project I/Arduino Project/main.ino"
            = "Unrecognized shock arguement, must use set/get/help";
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
    const static char str[] 
# 310 "/home/pi/Air Ride Project I/Arduino Project/main.ino" 3
   __attribute__((__progmem__)) 
# 310 "/home/pi/Air Ride Project I/Arduino Project/main.ino"
            = "usage: shock [0..3 / all] get/set pressure/height [setpoint if 'set']\n				setting pressure or height will also set system to that mode\n				'all' sets all shocks to same setting/mode";


    Serial.println(str);
   }
   else if (!strcmp(argv[1], "compressor"))
   {
    const static char str[] 
# 317 "/home/pi/Air Ride Project I/Arduino Project/main.ino" 3
   __attribute__((__progmem__)) 
# 317 "/home/pi/Air Ride Project I/Arduino Project/main.ino"
            = "compressor commands: \n				 set: set 'run' or 'idle' pressure (float psi), usage: compressor set <run/idle> <psivalue>\n				 get 'run', 'idle', or 'current' setpoint pressure, or specify 'state' to get run mode <running, idling, stopped>\n				 start: start compressor holding run pressure\n				 stop: stop compressor cycle\n				 idle: start compressor in idle mode\n				 bleed: open bleed valve and drop pressure to 0";






    Serial.println(str);
   }
   else
   {
    const static char str[] 
# 328 "/home/pi/Air Ride Project I/Arduino Project/main.ino" 3
   __attribute__((__progmem__)) 
# 328 "/home/pi/Air Ride Project I/Arduino Project/main.ino"
            = "Sorry, help command is not yet implemented.";
    debug_log(str);
   }
  }
  else
  {
   const static char str[] 
# 334 "/home/pi/Air Ride Project I/Arduino Project/main.ino" 3
  __attribute__((__progmem__)) 
# 334 "/home/pi/Air Ride Project I/Arduino Project/main.ino"
           = "unrecognized serial command. type 'help' for more info";
   Serial.println(str);
  }
 }
}
