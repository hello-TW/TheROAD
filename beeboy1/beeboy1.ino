#include <SoftwareSerial.h>

#define RX 3
#define TX 2

#define BEE_TURN 125   //beeboy - arrive and change turn code
#define BEE_TRAPED 123 //beeboy - arrive trap pos code

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
  int i,not_command = 0;
  int start_flag = 0;
  if(mySerial.available()){            /////// Serial -> mySerial
    Serial.println("start");
    trm = 125;
    not_command = 0;
    command = mySerial.readString();
    Serial.print("read : ");
    Serial.println(command);
    command_length = command.length(); // null문자 유무 따라서
    Serial.print("  length : ");
    Serial.println(command_length);
  
    for(i = 0; i < command_length; i++){
      Serial.println(command[i]);      
      switch(command[i]){
        case 'U': 
          adjust = UP - nsee;
          break;
        case 'D': 
          adjust = DOWN - nsee; 
          break;
        case 'R': 
          adjust = RIGHT - nsee;
          break;
        case 'L': 
          adjust = LEFT - nsee; 
          break;
        case 'e':
          trm = 0;
        case 'r':
          adjust = UP - nsee;
          start_flag = 1;
          Serial.println("adjust u");
          break;
        case 't':
          trm = BEE_TRAPED;
        default:
          not_command = 1;
          break;
      }
      if(not_command){
        break;
      }
      Serial.print("adjust: ");
      Serial.println(adjust);
      
      switch(adjust){
        case 2:
        case -2:
          moving(LOW,HIGH,71,70);
          if(nsee == LEFT) delay(1373);
          else if(nsee == RIGHT) delay(1380);
          else delay(1380);
          moving(LOW,LOW,0,0);delay(150);
          break; 
        case 1:
        case -3:// TURN right
          moving(HIGH,LOW,70,71);
          if(nsee == RIGHT || nsee == DOWN || nsee == UP) delay(695);
          else delay(700);
          moving(LOW,LOW,0,0);delay(150);
          break;
        case 3:
        case -1:// TURN LEFT
          moving(LOW,HIGH,70,69);
          if(nsee == RIGHT) delay(713);
          else if (nsee == LEFT) delay(725);
          else delay(715);
          moving(LOW,LOW,0,0);delay(150);
          break;
      }
      nsee += adjust; 
      if(nsee >= 4) nsee -= 4;
      else if(nsee <= -1) nsee += 4; //// 추가된 코드!!
      if(start_flag) start_flag = 0;
      else {
        moving(HIGH,HIGH,81,80);
        if(nsee == DOWN){
          delay(960);
        }
        else if (nsee == LEFT) delay(905);
        else delay(925);  
       
        //855
        moving(LOW,LOW,0,0);
        delay(150);
      }
    }
    mySerial.write(trm); // 종료
    Serial.println();  
  }
}


void moveForward()
{ 
  moving(HIGH,HIGH,81,80);
  delay(1015);//855
  moving(LOW,LOW,0,0);delay(150);
}
void turnRight()
{
  moving(HIGH,LOW,70,71);
  delay(720);
  moving(LOW,LOW,0,0);delay(150);
}
void turnLeft()
{
  moving(LOW,HIGH,70,69);
  delay(715);
  moving(LOW,LOW,0,0);delay(150);
}
void turnOpposite()
{
  moving(LOW,HIGH,71,70);
  delay(1395);
  moving(LOW,LOW,0,0);delay(150);
}

void moving(int dirl, int dirr, int pwml, int pwmr)
{
  digitalWrite(dir_l,dirl);
  digitalWrite(dir_r,dirr);
  analogWrite(pwm_l,pwml);
  analogWrite(pwm_r,pwmr);
}
