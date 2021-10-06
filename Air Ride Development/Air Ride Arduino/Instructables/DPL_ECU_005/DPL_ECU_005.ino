/********************************************
************************************************
****************************************************
********************************************************
THIS SOFTWARE WAS CREATED BY XTREAMERPT FROM XTREAMFIX
YOU CAN CHANGE IT AS YOU LIKE AND REDISTRIBUTE FOR FREE 
JUST KEEP MY COMPANY - NAME MENTION IN THE CODE
IF YOU HAVE ANY QUESTION YOU CAN CONTACT ME AT XTREAMFIXPT@GMAIL.COM
********************************************************************
***********************************************************************
*************************************************************************
****************************************************************************/



#include <SoftwareSerial.h>

//Variables for com with apk
boolean depurOnce = true;
boolean depurOK = false;
int passR;
int pass = 4401;
char depuracao = "E";


//************ECU Software Version****************
double versao = 0.5;



//Bluetooth Module pins
SoftwareSerial bluet(11,10); // RX | TX
int comIn, comOut, comTest;


//Some stuff i was trying to implement
const int heartBeat = 5000;
unsigned long tempoPassou;


//Variable trying to implement
int apontadorMenu = 0;


/*starComIn & stopComIn = Comands received by apk
 * ***  StartComIn  ****** stopComIn ****
 * ***Up*******Stop**
 * 0 = Sobe Frente Esq <-//-> 10 = Stop
 * 1 = Sobe Frente Drt <-//-> 11 = Stop
 * 2 = Sobe Tras Esq   <-//-> 12 = Stop
 * 3 = Sobe Tras Esq   <-//-> 13 = Stop
 * ***Down
 * 4 = Desce Frente Esq <-//-> 14 = Stop
 * 5 = Desce Frente Drt <-//-> 15 = Stop
 * 6 = Desce Tras Esq   <-//-> 16 = Stop
 * 7 = Desce Tras Drt   <-//-> 17 = Stop
*/
const int startComIn[] = { 0, 1, 2, 3,
                      4, 5, 6, 7};
const int stopComIn [] = { 10, 11, 12, 13, 
                     14, 15, 16, 17};

/* Solenoids pins */
const int digiStart[] = { 2, 3, 4, 5, 
                    6, 7, 8, 9};
const int digiStartCount = 8;


// buttons analog pin
const int buttonManual = A1;

//Variables for controling button menu
bool centerPress = false;
bool upPress = false;
bool downPress = false;
//************************************************************************************************

//Variable for extra Mode
boolean modoExtra = false;

//***********************************************************************

void setup()
{
  Serial.begin(19200);
  bluet.begin(19200);

  //define the solenoids pins
  for (int thisPin = 0; thisPin < digiStartCount; thisPin++){
    pinMode(digiStart[thisPin], OUTPUT); 
    digitalWrite(digiStart[thisPin], LOW);
  }

  
  //not in use
  /*//desliga os pinos das electroválvulas para evitar arranque inesperado
  for (int thisPin = 0; thisPin < digiStartCount; thisPin++){
    digitalWrite(digiStart[thisPin], LOW); 
  }*/

  //Just to add values not used by the hardware so there are no inexpected starts
  comIn = 99;
  comOut = 99;
  comTest = 99;
  
  
}

//Main
void loop(){
  
  receiveRem();
  manual();
  receiveRem();
  if (comIn >= 0 && comIn <= 37) {
    scanInRem();
  }
  receiveRem();
  extra();
  /*unsigned long tempoActual = millis();
  if ((unsigned long)(tempoActual - tempoPassou) >= heartBeat) {
    sendRem("OK*");
    Serial.print("OK*");
    tempoPassou = tempoActual;
  }*/

}

//Apk extra mode
void extra (){

  if (comIn >= 40 && comIn <= 50){
     digitalWrite(digiStart[4], HIGH); //d
     digitalWrite(digiStart[5], HIGH); //d
     digitalWrite(digiStart[6], HIGH); //d
     digitalWrite(digiStart[7], HIGH); //d
     delay(1500);
     modoExtra = true;
     Serial.println ("Extra Selected");
     digitalWrite(digiStart[4], LOW); 
     digitalWrite(digiStart[5], LOW); 
     digitalWrite(digiStart[6], LOW); 
     digitalWrite(digiStart[7], LOW); 
  }
  
  while (modoExtra == true){
    receiveRem();

    if (comIn == 41){ // Up FR & RR - Down FL & RL
      
      digitalWrite(digiStart[1], HIGH); //S
      digitalWrite(digiStart[3], HIGH); //S
      digitalWrite(digiStart[4], HIGH); //D
      digitalWrite(digiStart[6], HIGH); //D
      
      digitalWrite(digiStart[0], LOW); 
      digitalWrite(digiStart[2], LOW); 
      digitalWrite(digiStart[5], LOW); 
      digitalWrite(digiStart[7], LOW); 
    }else if (comIn == 42){ // Up FL & RL - Down FR & RR
      digitalWrite(digiStart[0], HIGH); //s
      digitalWrite(digiStart[2], HIGH); //s
      digitalWrite(digiStart[5], HIGH); //d
      digitalWrite(digiStart[7], HIGH); //d
      
      digitalWrite(digiStart[1], LOW); 
      digitalWrite(digiStart[3], LOW); 
      digitalWrite(digiStart[4], LOW); 
      digitalWrite(digiStart[6], LOW);
    }else if (comIn == 43){ //Up F - Down R
      digitalWrite(digiStart[0], HIGH); //s
      digitalWrite(digiStart[1], HIGH); //s
      digitalWrite(digiStart[6], HIGH); //d
      digitalWrite(digiStart[7], HIGH); //d

      digitalWrite(digiStart[2], LOW); 
      digitalWrite(digiStart[3], LOW); 
      digitalWrite(digiStart[4], LOW); 
      digitalWrite(digiStart[5], LOW);
    }else if (comIn == 44){ //Up R - Down F
      digitalWrite(digiStart[2], HIGH); //s
      digitalWrite(digiStart[3], HIGH); //s
      digitalWrite(digiStart[4], HIGH); //d
      digitalWrite(digiStart[5], HIGH); //d

      digitalWrite(digiStart[1], LOW); 
      digitalWrite(digiStart[0], LOW); 
      digitalWrite(digiStart[6], LOW); 
      digitalWrite(digiStart[7], LOW);
    }else if (comIn == 45){ //Down FL
      digitalWrite(digiStart[1], HIGH); //s
      digitalWrite(digiStart[2], HIGH); //s
      digitalWrite(digiStart[3], HIGH); //s
      digitalWrite(digiStart[4], HIGH); //d

      digitalWrite(digiStart[0], LOW); 
      digitalWrite(digiStart[5], LOW); 
      digitalWrite(digiStart[7], LOW); 
      digitalWrite(digiStart[6], LOW);
    }else if (comIn == 46){ //Down FR
      digitalWrite(digiStart[0], HIGH); //s
      digitalWrite(digiStart[2], HIGH); //s
      digitalWrite(digiStart[3], HIGH); //s
      digitalWrite(digiStart[5], HIGH); //d

      digitalWrite(digiStart[1], LOW); 
      digitalWrite(digiStart[4], LOW); 
      digitalWrite(digiStart[7], LOW); 
      digitalWrite(digiStart[6], LOW);
    }else if (comIn == 47){ //Down RL
      digitalWrite(digiStart[0], HIGH); //s
      digitalWrite(digiStart[1], HIGH); //s
      digitalWrite(digiStart[3], HIGH); //s
      digitalWrite(digiStart[6], HIGH); //d

      digitalWrite(digiStart[2], LOW); 
      digitalWrite(digiStart[4], LOW); 
      digitalWrite(digiStart[5], LOW); 
      digitalWrite(digiStart[7], LOW);
    }else if (comIn == 48){ //Down RR
      digitalWrite(digiStart[0], HIGH); //s
      digitalWrite(digiStart[1], HIGH); //s
      digitalWrite(digiStart[2], HIGH); //s
      digitalWrite(digiStart[7], HIGH); //d

      digitalWrite(digiStart[3], LOW); 
      digitalWrite(digiStart[4], LOW); 
      digitalWrite(digiStart[5], LOW); 
      digitalWrite(digiStart[6], LOW);
     
    }else if (comIn == 51){ //All DOwn
      digitalWrite(digiStart[4], HIGH); //d
      digitalWrite(digiStart[5], HIGH); //d
      digitalWrite(digiStart[6], HIGH); //d
      digitalWrite(digiStart[7], HIGH); //d

      digitalWrite(digiStart[0], LOW); 
      digitalWrite(digiStart[1], LOW); 
      digitalWrite(digiStart[2], LOW); 
      digitalWrite(digiStart[3], LOW);
    }else if(comIn == 55 || comIn == 99){
      digitalWrite(digiStart[4], HIGH); //d
      digitalWrite(digiStart[5], HIGH); //d
      digitalWrite(digiStart[6], HIGH); //d
      digitalWrite(digiStart[7], HIGH); //d
      digitalWrite(digiStart[0], LOW); 
     digitalWrite(digiStart[1], LOW); 
     digitalWrite(digiStart[2], LOW); 
     digitalWrite(digiStart[3], LOW); 
      delay(1500);
      Serial.println ("Exit extra mode");
     
     digitalWrite(digiStart[4], LOW); 
     digitalWrite(digiStart[5], LOW); 
     digitalWrite(digiStart[6], LOW); 
     digitalWrite(digiStart[7], LOW); 
     delay(5);
     modoExtra = false;
    }
    
  }
 
}



//Manual mode for buttons
void manual(){
  int b = analogRead(buttonManual);
  
  if (comIn == 99 ){

  
  if (b < 275 && b > 125){
    
    if (centerPress == true){
      
        Serial.print ("Rise Back ");
        Serial.print ("Value analog = ");
        Serial.println(b);
        digitalWrite(digiStart[2], HIGH);
        digitalWrite(digiStart[3], HIGH);
        b = analogRead(buttonManual);
      
      
    } else if (centerPress == false){
      
      Serial.print ("Rise Front ");
      Serial.print ("Value analog = ");
      Serial.println(b);
      digitalWrite(digiStart[0], HIGH);
      digitalWrite(digiStart[1], HIGH);
      
    }
  }else if (b < 370 && b > 280){
          Serial.println ("Change Mode ");
          if (centerPress == false) {
            
            centerPress = true;
            digitalWrite(digiStart[0], LOW);
            digitalWrite(digiStart[1], LOW);
            digitalWrite(digiStart[4], LOW);
            digitalWrite(digiStart[5], LOW);
            delay(200);
            
          }else if (centerPress == true){
            
            centerPress = false;
            digitalWrite(digiStart[2], LOW);
            digitalWrite(digiStart[3], LOW);
            digitalWrite(digiStart[6], LOW);
            digitalWrite(digiStart[7], LOW);
            delay(200);
            
          }
        }else if (b < 500 && b > 380) {
                
                if (centerPress == true){
                  
                  Serial.print ("Lower back ");
                  Serial.print ("value analog = ");
                  Serial.println(b);
                  digitalWrite(digiStart[6], HIGH);
                  digitalWrite(digiStart[7], HIGH);
                  
                }else if (centerPress == false){
                  
                    Serial.print ("Lower Front ");
                    Serial.print ("value analog = ");
                    Serial.println(b);
                    digitalWrite(digiStart[4], HIGH);
                    digitalWrite(digiStart[5], HIGH);
                }
        }
  
  
  delay(50);
  digitalWrite(digiStart[2], LOW);
  digitalWrite(digiStart[3], LOW);
  digitalWrite(digiStart[6], LOW);
  digitalWrite(digiStart[7], LOW);
  digitalWrite(digiStart[0], LOW);
  digitalWrite(digiStart[1], LOW);
  digitalWrite(digiStart[4], LOW);
  digitalWrite(digiStart[5], LOW);
  
    
  }
}

//Receive from Android
void receiveRem () {

  if (bluet.available()){
    
      comIn =  bluet.read();
     /* bluet.print("NOK*");*/
        Serial.print("Received - ");  
        Serial.print(comIn); 
        Serial.println(""); 

      
      delay(10);
    
    
  }
  
}



//Process what is received from android
void scanInRem() {
  


  /******************Ciclos para comando de Roda a Roda************************/
 if(comIn <= 7){
  for (int x = 0; x < digiStartCount; x++) {
        
        if (comIn == startComIn[x]){
          digitalWrite(digiStart[x], HIGH);
          
        }
      } 
  }else if(comIn >= 10 && comIn <= 17){
      for (int x = 0; x < digiStartCount; x++) {
        if (comIn == stopComIn[x]){
          digitalWrite(digiStart[x], LOW);
        
         }
      }
      
        /****************** Ciclos para comandos para operação eixo a eixo ************************/
        }else if(comIn >= 18 && comIn <= 40 ){
            if (comIn == 30) {
              digitalWrite(digiStart[0], HIGH);
              
              digitalWrite(digiStart[1], HIGH);
            }else if (comIn == 31) {
              digitalWrite(digiStart[0], LOW);
              
              digitalWrite(digiStart[1], LOW);
            }else if (comIn == 34) {
              digitalWrite(digiStart[4], HIGH);
              
              digitalWrite(digiStart[5], HIGH);
            }else if (comIn == 35){
              digitalWrite(digiStart[4], LOW);
              delay(1);
              digitalWrite(digiStart[5], LOW);
            }else if (comIn == 32){
              digitalWrite(digiStart[2], HIGH);
              
              digitalWrite(digiStart[3], HIGH);
            }else if (comIn == 33){
              digitalWrite(digiStart[2], LOW);
              
              digitalWrite(digiStart[3], LOW);
            }else if (comIn == 36){
              digitalWrite(digiStart[6], HIGH);
              
              digitalWrite(digiStart[7], HIGH);
            }else if (comIn == 37){
              digitalWrite(digiStart[6], LOW);
              
              digitalWrite(digiStart[7], LOW);
            }
         }
}

/*void depurMode () {

  if(depurOnce){
    Serial.println("Wish to start verbose Mode? (Y/N)");
    
      if (Serial.available()){
        depuracao = Serial.read();
          if (depuracao == "Y" || depuracao == "y"){
              Serial.println("Verbose Mode Selected");
              Serial.println("Please insert Password");
              passR = Serial.read();
              if (passR == pass){
                depurOK = true;
                Serial.println("Password Accepted, Welcome.");
                Serial.print("ECU Booting with version - ");
                Serial.print(versao);
              }
            }else if(depuracao == "N" || depuracao == "n"){
                    depurOK = false;
                   }else if (depuracao != "Y" || depuracao != "y" || depuracao != "N" || depuracao != "n"){
                           Serial.println("Wrong key selected, ECU booting...");
                         }
      }
    delay(1000);
    depurOnce = !depurOnce;  
  }
}*/

