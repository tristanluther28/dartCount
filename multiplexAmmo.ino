/* Multiplex Ammo Counter
 *  
 *  This sketch was written by:
 *  Tristan Luther
 *  November 20, 2017
 *  
 *  Here's the code that is paired with my 
 *  ammo counter schematic for HvZ
 */
//********************************************Variables****************************************************
//*********************************************************************************************************
int segPins[] = {4, 5, 9, 6, 8, 3, 2, 7 }; //pin setup from the ATMEGA328 to the seven-segment display 
int displayPins[] = {11, 10};   // pin 10 controls D0, pin 11 controls D1
int displayBuf[2];  //display buffer
int buttonState = 0; //  condition of the trigger pull sensor
int magState = 0; //  condition of the magazine sensor
int modeSet = 0;  //  the setting mode between magSizes and ammoCountdown
int magSizes[] = {6, 10, 12, 18, 35 };  //  add more numbers to change the different ammo countdowns 
int magSize = 0; //  a default mag size that will change with every trigger pull
int startup = 0;  //  used to intialize the magSize to the seven-segment displays
int value;  //  used for fade-in/fade-out when mag size is zero
int i = 0;  // used for moving through the magSizes array
int tensDigit = magSize / 10;  //  gives the tens digit of magSize
int onesDigit = magSize % 10;  //  gives the ones digit of magSize
int fullMag = 6; //  a default full mag is 6 darts, this will only change in the magSizes mode
int modeSwitch = A0;  // switch input for switching modes
const int buttonPin = 12; //  sensor input for counting trigger pulls 
const int reloadButton = 13; //  sensor input for detecting magazine
byte segCode[10][8] = {
// 7 segment code table
//  a  b  c  d  e  f  g  .
  //Common Cathode Array
  { 1, 1, 1, 1, 1, 1, 0, 0},  // 0
  { 0, 1, 1, 0, 0, 0, 0, 0},  // 1
  { 1, 1, 0, 1, 1, 0, 1, 0},  // 2
  { 1, 1, 1, 1, 0, 0, 1, 0},  // 3
  { 0, 1, 1, 0, 0, 1, 1, 0},  // 4
  { 1, 0, 1, 1, 0, 1, 1, 0},  // 5
  { 1, 0, 1, 1, 1, 1, 1, 0},  // 6
  { 1, 1, 1, 0, 0, 0, 0, 0},  // 7
  { 1, 1, 1, 1, 1, 1, 1, 0},  // 8
  { 1, 1, 1, 1, 0, 1, 1, 0}   // 9
  //Common Anode Array
  /*{ 0, 0, 0, 0, 0, 0, 1, 1},  // 0
  { 1, 0, 0, 1, 1, 1, 1, 1},  // 1
  { 0, 0, 1, 0, 0, 1, 0, 1},  // 2
  { 0, 0, 0, 0, 1, 1, 0, 1},  // 3
  { 1, 0, 0, 1, 1, 0, 0, 1},  // 4
  { 0, 1, 0, 0, 1, 0, 0, 1},  // 5
  { 0, 1, 0, 0, 0, 0, 0, 1},  // 6
  { 0, 0, 0, 1, 1, 1, 1, 1},  // 7
  { 0, 0, 0, 0, 0, 0, 0, 1},  // 8
  { 0, 0, 0, 0, 1, 0, 0, 1}   // 9
  */
};
//*********************************Set-Up***********************************************
//**************************************************************************************
void setup()
{
  for (int i=0; i < 8; i++)
  {
    pinMode(segPins[i], OUTPUT);
  }
  pinMode(displayPins[0], OUTPUT);
  pinMode(displayPins[1], OUTPUT);
  pinMode(buttonPin,INPUT_PULLUP);
  pinMode(reloadButton,INPUT_PULLUP);
  pinMode(modeSwitch,INPUT);

  displayBuf[1] = tensDigit;    // initializes the display
  displayBuf[0] = onesDigit;
  refreshDisplay(displayBuf[1],displayBuf[0]);

  Serial.begin(9600);
   }
//******************************Main Loop***********************************************
//**************************************************************************************
void loop()
{
  refreshDisplay(displayBuf[1],displayBuf[0]);
  buttonState = digitalRead(buttonPin); //  get reading of trigger condition
  magState = digitalRead(reloadButton); //  get reading of magazine condition
  modeSet = analogRead(modeSwitch);
  if (modeSet == LOW)
  {
    if(magState == LOW) //  if the magazine is in the gun then the number will display the number in the mag
  {
    startup++; 
    
    if(debounce(buttonState) == LOW && startup > 1) //  if the trigger sensor is active then roundShot() is activated
    {
      changeNumber();
    }
    else if(debounce(buttonState) == HIGH && startup > 1)
    {
      tensDigit = magSize / 10;
      onesDigit = magSize % 10;
      displayBuf[0] = onesDigit;
      displayBuf[1] = tensDigit;
      Serial.println((tensDigit * 10) + onesDigit);
      fadeSegments();
      
    }
    else  //  if the number being displayed is less than zero then the magazine is reset to nine
    {
      magSize = fullMag; 
    }
  }
  else  //  if no mag is inserted then the display shows zero
  {
    onesDigit = 0;
    tensDigit = 0;
    magSize = 0;
    startup = 0;
    fadeSegments();
  }
  displayBuf[0] = onesDigit;
  displayBuf[1] = tensDigit;
  Serial.println(magSize);
  }
  else 
  { 
      if(buttonState == LOW && i < 5) //change the 5 if the magSizes array has more than 5 elements
       {
        i++;
        delay(250);
       }
       else if(i >= 5) //change the 5 if the magSizes array has more than 5 elements
       {
         i = 0;
       }
       else
       {
        fullMag = magSizes[i];
        magSize = fullMag;
        tensDigit = magSize / 10;
        onesDigit = magSize % 10;
        displayBuf[0] = onesDigit;
        displayBuf[1] = tensDigit;
        Serial.println(magSize);
        Serial.println(i);
       }
  }
}
//**************************************Methods********************************************
//*****************************************************************************************
void refreshDisplay(int digit1, int digit0)
{
  digitalWrite(displayPins[0], HIGH);  // displays digit 0 (least significant)
  digitalWrite(displayPins[1],LOW );
  setSegments(digit0);
  delay(5);
  digitalWrite(displayPins[0],LOW);    // then displays digit 1
  digitalWrite(displayPins[1], HIGH);
  setSegments(digit1);
  delay(5);
}
bool debounce(bool last){
  bool current = digitalRead(buttonPin);
  if(last != current){
    delay(100);
    current = digitalRead(buttonPin);
  }
  return current;
}
void fadeSegments()
{
  if (tensDigit == 0 && onesDigit == 0)
      {
        for(value = 0 ; value <= 255; value+=5) // fade in (from min to max) 
            { 
              analogWrite(11, value);           // sets the value (range from 0 to 255) 
              analogWrite(10, value);           // sets the value (range from 0 to 255) 
              delay(10);                            // waits for 30 milli seconds to see the dimming effect 
            } 
          for(value = 255; value >=0; value-=5)   // fade out (from max to min) 
            { 
              analogWrite(11, value);           // sets the value (range from 0 to 255) 
              analogWrite(10, value);           // sets the value (range from 0 to 255) 
              delay(10); 
            }  
      }
}
void setSegments(int n)
{
  for (int i=0; i < 8; i++)
  {
    digitalWrite(segPins[i], segCode[n][i]);
  }
}
void changeNumber()
{
  if( magSize > 9 )
  {
    magSize--;
    tensDigit = magSize / 10;
    onesDigit = magSize % 10;
    //displayBuf[0] = onesDigit;                // send the updated count to the display buffer
    //displayBuf[1] = tensDigit;
    delay(250);
  }
  else if( magSize <= 9 && magSize > 0 )
  {
    magSize--;
    tensDigit = 0; // Set the first digit to 0
    onesDigit = magSize;
    //displayBuf[0] = onesDigit;                // send the updated count to the display buffer
    //displayBuf[1] = tensDigit;
    delay(250);
  }
  else 
  {
    tensDigit = 0; // Set the first digit to 0
    onesDigit = 0;
    //displayBuf[0] = onesDigit;                // send the updated count to the display buffer
    //displayBuf[1] = tensDigit;
  }
}

