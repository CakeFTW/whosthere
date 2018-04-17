// tutotial for our servo 
// https://playground.arduino.cc/Learning/SingleServoExample

#define greenLED 7
#define redLED 6
#define pizPin A0
#define KNOCKTHRESHOLD 30
#define KNOCKPERIOD 75
int lastReading = 0;
int currentReading = 0;
bool isOpen = false; 
bool knocked = false;
bool offCooldown = true;
unsigned long knockCooldown = 0;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  currentReading = analogRead(pizPin);

  if(currentReading > KNOCKTHRESHOLD && !knocked && offCooldown){
    isOpen = !isOpen;
    knocked = true;
    Serial.println("lock changed state");
  }
  if(knocked && currentReading < KNOCKTHRESHOLD && lastReading < KNOCKTHRESHOLD ){
    Serial.println("cooldown in effect");
    knockCooldown = millis();
    knocked = false;
    offCooldown = false;
  }
  if(millis() >= (knockCooldown + KNOCKPERIOD)  && offCooldown == false){
    offCooldown = true;
    Serial.println("cooldown in done");
  }
  
  delay(1);
  if(isOpen){
    digitalWrite(greenLED, HIGH);
    digitalWrite(redLED, LOW);
  }else{
    digitalWrite(greenLED, LOW);
    digitalWrite(redLED,HIGH);
  }
  lastReading = currentReading;
  if(currentReading > 5){
    Serial.println(currentReading);
  }
}
