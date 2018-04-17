// tutotial for our servo 
// https://playground.arduino.cc/Learning/SingleServoExample
#include <Servo.h>

Servo myServo;  // create servo object to control a servo
// twelve servo objects can be created on most boards
#define greenLED 7
#define redLED 6
#define pizPin A0
#define conPin 2
#define KNOCKTHRESHOLD 500
#define RESETTHRESHOLD 5
#define KNOCKPERIOD 100
#define RECORDDURATION 5000
int lastReading = 0;
int currentReading = 0;
bool knocked = false;
bool offCooldown = true;
unsigned long knockCooldown = 0;
unsigned long timeStamp = 0;
unsigned long timer; 


class KnockMatchLock{
  public:
  unsigned long key[50];
  unsigned long input[50];
  int inputCounter = 0;
  int keyCounter = 0;
  unsigned long keyStart = 0;
  unsigned long inputStart = 0;
  
  int precision = 50;
  bool isLocked = false;
  bool recording = false;
  bool keyRecorded = false;
  bool boxClosed = false;
  
  KnockMatchLock::KnockMatchLock(int _PRECISION){
    precision = _PRECISION;
    for(int i = 0; i < 50; i++){
      input[i] = 0;
      key[i] = 0;
    }
  };

  void updateStates(unsigned long stamp){
    if(recording && !keyRecorded){
      if(stamp > keyStart + RECORDDURATION){
        recording = false;
        keyRecorded = true; 
      }
    }
    if(stamp > inputStart + RECORDDURATION && inputCounter > 0 ){
      //reset
      for(int i = 0; i < 50; i++ ){
        input[i] = 0;
      }
      inputCounter = 0;
    }
    // check lid status
    if(digitalRead(conPin) == 1){
      boxClosed = true;
      lock();
    }else{
      boxClosed = false;
    }
    if(isLocked){
      digitalWrite(greenLED, LOW);
      digitalWrite(redLED, HIGH);
    }else{
      digitalWrite(greenLED, HIGH);
      digitalWrite(redLED, LOW);
    }
  }

  void knock(unsigned long &stamp){
    // if open and lid closed and no key has been recorded start the recording
    if (boxClosed && !keyRecorded){
      recording = true;
    }
    if(!recording){
      addIn(stamp);
    }else{
      addKey(stamp);
    }
  }
  
  void addKey(unsigned long &stamp){
    if(keyCounter == 0){
      key[0] = 0;
      keyStart = stamp;
      keyCounter++;
    }else{
      key[keyCounter] = stamp - keyStart;
      keyCounter++;
    }
  }

  void addIn(unsigned long &stamp){
    if(inputCounter == 0) {
      inputStart = stamp;
      inputCounter++;
    }else{
      input[inputCounter] = stamp - inputStart;
      inputCounter++;
      testMatch();
    }
  }

  void printLockData(){
    Serial.println( " lock states: LOCKED: " + String(isLocked) + " RECORDING: " + String(recording) + " KEYRECORDED: " + String(keyRecorded) + " LIDCLOSED: " + String(boxClosed) + "InputStart" + String(inputStart));
    for(int i = 0 ; i < keyCounter; i++){
      Serial.print(i);
      Serial.print( "    " );
    }
    Serial.println();
    for(int i = 0 ; i < keyCounter; i++){
      Serial.print(key[i]);
      Serial.print( " " );
    }
    Serial.println();
    for(int i = 0 ; i < keyCounter; i++){
      Serial.print(input[i]);
      Serial.print( " " );
    }
    Serial.println();
  }

  bool getLocked(){
    return isLocked;
  }

  void lock(){
    isLocked = true;
    myServo.write(90);
    
  }
  void unlock(){
    isLocked = false;
    myServo.write(0);
  }

  bool testMatch(){
    if(keyCounter == 0 || inputCounter < keyCounter){
      return false;
      }
    bool matching = true;
    for(int i = 1; i < keyCounter; i++){
      int howClose = abs(input[i] - key[i]);
      if(howClose > precision){
        matching = false;
        Serial.println("FALSE MATCH" + String(howClose));
        Serial.println(String(key[i]) + " vs " + String(input[i]));
      }
    }
    if(matching){
      reset();
      Serial.println("UNLOCKED!!!!!");
      
    }else{
      Serial.println("lock test failed");
    }
    return matching;
  }

  void reset(){
    //resets all states to default and opens the lock
    *(this) = KnockMatchLock(precision); 
    unlock();
    delay(250);
  }
};

KnockMatchLock lock = KnockMatchLock(150);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  myServo.attach(9);
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(conPin, HIGH);
  Serial.println(lock.precision);
  lock.reset();
}

void loop() {
  // put your main code here, to run repeatedly:
  currentReading = analogRead(pizPin);
  timeStamp = millis();
  lock.updateStates(timeStamp);
  if(timeStamp> timer ){
    timer = timeStamp + 2000;
    lock.printLockData();
  }

  if(currentReading > KNOCKTHRESHOLD  && !knocked && offCooldown){
    knocked = true;
    lock.knock(timeStamp);
    //Serial.println("lock changed state");
  }
  if(knocked && currentReading < RESETTHRESHOLD && lastReading < RESETTHRESHOLD){
    //Serial.print("cooldown in effect");
    knockCooldown = timeStamp;
    knocked = false;
    offCooldown = false;
    //Serial.println(millis());
  }
  if(timeStamp >= (knockCooldown + KNOCKPERIOD) && offCooldown == false){
    offCooldown = true;
    //Serial.print("cooldown in done");
    //Serial.println(millis());
  }
  lastReading = currentReading;
}
