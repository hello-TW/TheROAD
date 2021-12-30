#include <SoftwareSerial.h>

#define RX 3
#define TX 2
SoftwareSerial mySerial(RX, TX);

int dir_l = 4;
int dir_r = 7;
int pwm_l = 6;
int pwm_r = 5; //// 모터구동

const int UP = 0;
const int RIGHT = 1; 
const int DOWN = 2;
const int LEFT = 3; //// 방향

char trm = 125;

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  pinMode(dir_l, OUTPUT);
  pinMode(dir_r, OUTPUT);
  pinMode(pwm_l, OUTPUT);
  pinMode(pwm_r, OUTPUT);
}

int nsee = 0; // 처음에 보고있는 방향
int adjust = 0; // 바꿔야될 방향

String command;
int command_length,len = 0;

void loop() { 
  /*
  if(mySerial.available()){
    int timer = mySerial.readString().toInt();
    Serial.println(timer);
    moving(LOW,HIGH,80,80);
    delay(timer);
  }
  */
  delay(3000);
  turnOpposite();
  moveForward();
  turnOpposite();
  moveForward();
  turnOpposite();
  moveForward();
}


void moveForward()
{ 
  moving(HIGH,HIGH,80,83);delay(900);//855 //950

  moving(LOW,LOW,0,0);delay(100);
}
void turnRight()
{
  moving(HIGH,LOW,80,80);
  
  delay(566);
  moving(LOW,LOW,0,0);delay(100);
}
void turnLeft()
{
  moving(LOW,HIGH,80,80);
  delay(564);
  moving(LOW,LOW,0,0);delay(100);
}
void turnOpposite()
{
  moving(LOW,HIGH,80,80);
  delay(1140);
  moving(LOW,LOW,0,0);delay(100);
}

void moving(int dirl, int dirr, int pwml, int pwmr)
{
  digitalWrite(dir_l,dirl);
  digitalWrite(dir_r,dirr);
  analogWrite(pwm_l,pwml);
  analogWrite(pwm_r,pwmr);
}
