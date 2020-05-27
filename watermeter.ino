#include <LiquidCrystal.h>
#include <EEPROM.h>

LiquidCrystal lcd(13, 12, 11, 10, 9, 8);



byte statusLed = 13;

byte sensorInterrupt = 0;
byte sensorPin = 2;
byte resetButtonPin = 5;
byte ledPin = 4;
byte raiseLimitPin = 6;
byte lowerLimitPin = 7;
byte calibrateButtonPin = 3;

boolean isCalibrating = false;



float calibrationFactor = 1;

int LitresAddress;
int MaxFlowAddress;
int calibrationFactorAddress;

int millitersCalibrate = 0;

volatile byte pulseCount;

unsigned int maxFlow;

float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;
unsigned int totalLitres;


unsigned long oldTime;
unsigned long btnTime;



void setup()
{

pinMode(resetButtonPin, INPUT_PULLUP);
pinMode(raiseLimitPin, INPUT_PULLUP);
pinMode(lowerLimitPin, INPUT_PULLUP);
pinMode(calibrateButtonPin, INPUT_PULLUP);
pinMode(14,OUTPUT);
digitalWrite(14,HIGH);



lcd.begin(16, 2);


lcd.print("Current Flow:");
lcd.setCursor(0, 1);


Serial.begin(9600);
Serial.print(EEPROM.read(8));


pinMode(statusLed, OUTPUT);
digitalWrite(statusLed, HIGH);

pinMode(sensorPin, INPUT);
digitalWrite(sensorPin, HIGH);

LitresAddress = 0;
MaxFlowAddress = 4;
calibrationFactorAddress = 8;
pulseCount = 0;
flowRate = 0.0;
flowMilliLitres = 0;
totalLitres = EEPROM.read(LitresAddress);
totalMilliLitres = totalLitres * 1000;
oldTime = 0;
maxFlow = EEPROM.read(MaxFlowAddress);
EEPROM.get( calibrationFactorAddress, calibrationFactor );



attachInterrupt(sensorInterrupt, pulseCounter, FALLING);

}


void loop()
{

lcd.setCursor(0, 0);
lcd.print("Current Flow: ");

if(digitalRead(resetButtonPin) == LOW){
  totalLitres = 0;
  totalMilliLitres = 0;
  EEPROM.put(LitresAddress,0);
  EEPROM.put(MaxFlowAddress,20);
  lcd.setCursor(0, 1);
  lcd.print("Reset ");
  digitalWrite(14,HIGH);
}

if(digitalRead(raiseLimitPin) == LOW && millis() - btnTime > 1000){
  maxFlow += 2;
  EEPROM.put(MaxFlowAddress,maxFlow);

  btnTime = millis();
}





if(digitalRead(lowerLimitPin) == LOW && millis() - btnTime > 1000){
  
  maxFlow -= 2;
  EEPROM.put(MaxFlowAddress,maxFlow);

  btnTime = millis();
}


 
  
if(digitalRead(calibrateButtonPin) == LOW && millis() - btnTime > 1000){
  btnTime = millis();
 Serial.println("CalcRead ");
  if(!isCalibrating){
    isCalibrating = true;
    Serial.println("Start Calibr");
    calibrationFactor = 1;
    millitersCalibrate = totalMilliLitres;
  } else {
    isCalibrating = false;
        Serial.println("FINISH Calibr");
        Serial. print(flowMilliLitres);
        Serial. print("\t");
        Serial. print(millitersCalibrate);
    calibrationFactor =(double) (totalMilliLitres -  millitersCalibrate) / 1000;
    EEPROM.put(calibrationFactorAddress,calibrationFactor);
    totalMilliLitres =  millitersCalibrate;
    millitersCalibrate = 0;
    Serial.print("CalFac = ");
    Serial.println(calibrationFactor);
  }
}

if (totalLitres > maxFlow) {

  analogWrite(ledPin, 255);
}else{
  analogWrite(ledPin, 0);
}
if((millis() - oldTime) > 1000)
{


detachInterrupt(sensorInterrupt);


flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;

oldTime = millis();


flowMilliLitres = (flowRate / 60) * 1000;


totalMilliLitres += flowMilliLitres;
if(totalMilliLitres / 1000 > totalLitres){
totalLitres = totalMilliLitres / 1000;
EEPROM.put(LitresAddress, totalLitres);
}

unsigned int frac;


Serial.print("Flow rate: ");
Serial.print(int(flowRate));
Serial.print("L/min");
Serial.print("\t");


Serial.print("CalFactor ");
Serial.print(calibrationFactor);
Serial.println("mL");
Serial.print("\t");
Serial.print(totalMilliLitres/1000);
Serial.print("L");

lcd.setCursor(0, 1);

lcd.print(totalMilliLitres);
lcd.print(" mL ");
lcd.print(totalMilliLitres/1000);
lcd.print("\x2F");
lcd.print(maxFlow);
lcd.print(" L");


pulseCount = 0;


attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
}
}


void pulseCounter()
{

pulseCount++;
}
