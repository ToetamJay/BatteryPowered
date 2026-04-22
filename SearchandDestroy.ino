//PINS AND VARIABLES
//initialize US sensor pins
#define tPin 12 //trig
#define ePin 11 //ehco

//US array
int numDists = 5; //length of distArray[]
int distArray[] = {0, 0, 0, 0, 0}; //integer array
int index = 0; //index to feed data into array
int reqNum = 2; //min. sum of the array needed to warrant attack
bool close = false;
//initalize IR sensors pins
#define IRPinL A0
#define IRPinR A1

//initialize left motor pins
#define INL1 3 //A01
#define INL2 4 //A02

//initialize right motor pins
#define INR1 8 //B01
#define INR2 9 //B02

// distance variable
int distance = 0;

//IR sensor value variables
float IRL = 0;//left
float IRR = 0; //right   

//PWM
#define PWML 6 //left 
#define PWMR 5 //right

bool rotateCW = true; //true spin means CW, false spin means CCW
bool facingOut = false; //determines if you are facing the outer edge based on if your front IR triggered
bool detected = false; //remembers if you detected opponent on previous scan

//important constants
float maxSpeed = 150; //max motor speed, max is 255
float atkSpeed = 255;
float detectVar = 500; //ring detection variable

unsigned long searchStartTime = 0;   // The "stopwatch" memory
int sweepTime = 600;                 // Milliseconds to sweep the first way (Adjust this!)
bool sweepingFirstDirection = false; // Tracks if the stopwatch is currently running


void setup() {
  // put your setup code here, to run once:
//5 Second delay timer
  long timerDelay = millis();
  distance = 0;
  index = 0;
  Serial.begin(9600);
  //US pins
  pinMode(tPin, OUTPUT);
  pinMode(ePin, INPUT);

  //IR Sensors
  pinMode(IRPinL, INPUT);
  pinMode(IRPinR, INPUT);

  //Motor initialization here
 //Left motor
  pinMode(INL1, OUTPUT);
  pinMode(INL2, OUTPUT);

 //Right motor
  pinMode(INR1, OUTPUT);
  pinMode(INR2, OUTPUT);
 
 //PWM
  analogWrite(PWML, maxSpeed);
  analogWrite(PWMR, maxSpeed);
 // start sumobot
  start();

  while(millis() - timerDelay < 3500){

  }//5-second delay
  analogWrite(PWML, atkSpeed);
  analogWrite(PWMR, atkSpeed);
  attack();
  delay(300);
}

void loop() {
  //collect and analyze data from sensors here
  delay(5); //why the delay here?
  distance = getDistance();
  IRL = getColor(IRPinL);
  IRR = getColor(IRPinR);
  //IRBack = getColor(IRPinB);

  distArray[index] = withinRange(distance, 40.0);
  close = isCloseEnough(distArray, numDists);
  index = (index + 1) % numDists;
  
//bot strategy here

  if(detectRing() == true){ //if detects edge of ring, reverse, then turn back to centre of ring
    //facingOut = true;
    analogWrite(PWML, atkSpeed);
    analogWrite(PWMR, atkSpeed);  
    ringResponse(); //ring response already has detect ring attached to it
    delay(250);
    
  }else{
    //could move if(index >= numDist) ... here? would ensure IR is ALWAYS checked first, then we get to attacking business?
    /*if(facingOut){
      spin();
      delay(250);
      facingOut = false; //-------------ADJUST AS NECESSARY, should be full 180
    }*/

    if(close == true){        //-------------------attack (regular) 
      //TEST MODE< TRY LOCKING ONTO OPPONENT
      detected = true;
      analogWrite(PWML, atkSpeed);
      analogWrite(PWMR, atkSpeed);
      attack();
    }else{                     //------------------------search
      if(detected == true){
        rotateCW = !rotateCW;           // Flip direction to try and catch them
        searchStartTime = millis();     // Click the stopwatch!
        sweepingFirstDirection = true;  // Tell the bot we are timing a sweep
        detected = false;               // Reset the detection flag
      }

      // 2. HAS OUR SWEEP TIMER RUN OUT?
      if(sweepingFirstDirection == true && (millis() - searchStartTime > sweepTime)){
        rotateCW = !rotateCW;           // Time's up! We guessed wrong. Flip the other way.
        sweepingFirstDirection = false; // Stop the stopwatch. Spin this way indefinitely.
      }

      analogWrite(PWML, maxSpeed);
      analogWrite(PWMR, maxSpeed);
      spin();
    }
    
  }

}

//------------------------Functions-----------------------------------------

// STATE METHODS
//attack the opponent
void attack(){
// move forward
  motorForward(INL1, INL2); //one of the motors are inverted, going forward
  motorForward(INR1, INR2); 

  //Serial.print("attacking");
}
void spin(){
  if(rotateCW){
    motorBackward(INL1, INL2); //one of the motors are inverted, going in a spinning motion 
    motorForward(INR1, INR2);
  }else{
    motorForward(INL1, INL2); 
    motorBackward(INR1, INR2);
  }
  
  //Serial.print("Spinning");
}

//initial state for beginning of round
void start(){
  distance = getDistance();
  IRL = getColor(IRPinL);
  IRR = getColor(IRPinR);
}

//bot's reponse to detecting the ring
void ringResponse(){
  motorBackward(INL1, INL2); 
  motorBackward(INR1, INR2);
  //Serial.print("Left IR:");
  //Serial.println(IRL);
  //Serial.print("Right IR:");
  //Serial.println(IRR);
}
//final state for end of round
void stop(){
  motorStop(INL1, INL2);
  motorStop(INR1, INR2);
}



// DATA COLLECTION/PROCESSING
//colelcts and sends data fron US sensor
float getDistance(){
  //digitalWrite(tPin, LOW);

  //delayMicroseconds(2);

  digitalWrite(tPin, HIGH);

  delayMicroseconds(10);

  digitalWrite(tPin, LOW);

  float duration = pulseIn(ePin, HIGH); 

  return (duration * 0.0343) / 2;
}

//detects white ring
bool detectRing(){
return IRR < detectVar || IRL < detectVar;
}

//detects color for IR sensors
float getColor(int pin){
return analogRead(pin);
}

//sets motors to move forward
void motorForward(int IN1, int IN2){
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
}

//sets motors to move backward
void motorBackward(int IN1, int IN2){
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
}

//stops motors
void motorStop(int IN1, int IN2){
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
}

//computed average
/*
float getAvg(float nums[], int length){
  float ans = 0.0;
  for(int i = 0; i < length; i++){
    ans = ans + nums[i];
  }

  return ans / length;
}*/

bool isCloseEnough (int vals[], int length){
  int tot = 0;
  //total the numbers in the array
  for(int i = 0; i < length; i++){
    tot = tot + vals[i];
  }

  if(tot > reqNum){
    return true;
  }else{
    return false;
  }

}

//converts distance to binary values
int withinRange(float val, float max){
  //ret 1 if val <= max, 0 otherwise
  if(val <= max || val > 700){
    return 1;
  }else{
    return 0;
  }
  
}

