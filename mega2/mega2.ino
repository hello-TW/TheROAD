#include <SoftwareSerial.h>

#define M2_START 124  //mega2 - wifi initialize end and start welcome2 code
#define M2_END 125    //mega2 - game end code
#define M2_RETURN 126 //mega2 - return and restart code
#define NOTE_TIME 1500 //Notification time interval when trapped

SoftwareSerial mySerial(10,11);

int pin_size;
int i;
char led_pin[9]={0,0,0,0,0,0,0,0,0};
char pin = 0;
const int pin_arr[4][8] = {
    {53, 45, 37, 29, 52, 44, 36, 28},
    {51, 43, 35, 27, 50, 42, 34, 26},
    {49, 41, 33, 25, 48, 40, 32, 24},
    {47, 39, 31, 23, 46, 38, 30, 22}};

void on(int pin_n);
void off(int pin_n);

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);

  for (int i = 22; i < 54; i++) {
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
  }
  welcome();
}


void loop() {
  while(1)
    if(mySerial.available() && mySerial.read() == M2_START)
      break;
  welcome2();
  while(1){
    if (mySerial.available()) {
      pin_size = mySerial.read();delay(1);
      if(pin_size == M2_END)
        break;
      else if(pin_size  == M2_START)
        continue;
      mySerial.readBytes(led_pin,pin_size);delay(1);
      
      Serial.print("pin_size : ");
      Serial.println(pin_size);
      Serial.print("led_pin : ");
  
      for(i = 0; i < pin_size; i++){
        Serial.print((int)led_pin[i]);
        Serial.print(", ");
        on(led_pin[i]);
      }
      Serial.println();
      delay(NOTE_TIME);
      for (i = 0; i < pin_size; i++) {
        off(led_pin[i]);
      }
    }
  }
  while(1){
    if (mySerial.available()){
      pin = mySerial.read();
      if(pin == M2_RETURN){
        break;
      }
      on(pin);
    }
  }
}


void welcome(){
  on(53);delay(40);
  on(45);delay(40);on(51);delay(40);
  on(49);delay(40);on(43);delay(40);on(37);delay(40);
  on(29);delay(40);on(35);delay(40);on(41);delay(40);on(47);delay(80);
  //mega1 호출
  on(39);delay(40);on(33);delay(40);on(27);delay(40);on(52);delay(40);
  on(44);delay(40);on(50);delay(40);on(25);delay(40);on(31);delay(240);
  on(23);delay(40);on(48);delay(40);on(42);delay(40);on(36);delay(40);
  on(28);delay(40);on(34);delay(40);on(40);delay(40);on(46);delay(360);
  on(38);delay(40);on(32);delay(40);on(26);delay(40);
  on(24);delay(40);on(30);delay(360);
  on(22);delay(560); 
  delay(100);// led 다키고 대기

  off(22);off(24);off(26);off(28);delay(100);
  off(30);off(32);off(34);off(36);delay(100);
  off(38);off(40);off(42);off(44);delay(100);
  off(46);off(48);off(50);off(52);delay(100);
  off(23);off(25);off(27);off(29);delay(100);
  off(31);off(33);off(35);off(37);delay(100);
  off(39);off(41);off(43);off(45);delay(100);  
  off(47);off(49);off(51);off(53);delay(100); 
  delay(100); ////////////////////////////////////////////////////

  on(53);on(51);on(49);on(47);delay(200);
  on(37);on(35);on(33);on(31);delay(200);
  on(52);on(50);on(48);on(46);delay(200);
  on(36);on(34);on(32);on(30);delay(200);
  on(45);on(43);on(41);on(39);delay(200);
  on(29);on(27);on(25);on(23);delay(200);
  on(44);on(42);on(40);on(38);delay(200);
  on(28);on(26);on(24);on(22);delay(200);
  delay(100);
  off(22);off(24);off(26);off(28);delay(100);
  off(30);off(32);off(34);off(36);delay(100);
  off(38);off(40);off(42);off(44);delay(100);
  off(46);off(48);off(50);off(52);delay(100);
  off(23);off(25);off(27);off(29);delay(100);
  off(31);off(33);off(35);off(37);delay(100);
  off(39);off(41);off(43);off(45);delay(100);  
  off(47);off(49);off(51);off(53);delay(100); 
  delay(100); ////////////////////////////////////////////////////
  
  

  for(int i = 22; i<=53; i++){
    on(i);
  }
  delay(1000);
  for(int i = 22; i<=53; i++){
    off(i);
  }
  //////////////////////////////////////////////////////////
  
}
void welcome2(){
  //1
  on(23);on(46);delay(200);

  //2
  off(23);off(46);
  on(31);on(33);on(25);on(48);on(40);on(38);delay(200);

  //3
  off(31);off(33);off(25);off(48);off(40);off(38);
  on(39);on(41);on(43);on(35);on(35);on(27);on(50);
  on(42);on(34);on(32);on(30);delay(200);

  //4
  off(39);off(41);off(43);off(35);off(35);off(27);
  off(50);off(42);off(34);off(32);off(30);
  on(47);on(49);on(51);on(53);on(45);on(37);on(29);
  on(52);on(44);on(36);on(28);on(26);on(24);on(22);delay(200);
  
  
  off(47);off(49);off(51);off(53);off(45);off(37);off(29);
  off(52);off(44);off(36);off(28);off(26);off(24);off(22);
  on(39);on(41);on(43);on(35);on(35);on(27);on(50);
  on(42);on(34);on(32);on(30);delay(200);

  off(39);off(41);off(43);off(35);off(35);off(27);
  off(50);off(42);off(34);off(32);off(30);
  on(31);on(33);on(25);on(48);on(40);on(38);delay(200);

  off(31);off(33);off(25);off(48);off(40);off(38);delay(50);
  on(23);on(46);delay(200);
  off(23);off(46);
}

void on(int pin_n) {
  digitalWrite(pin_n, HIGH);
}

void off(int pin_n) {
  digitalWrite(pin_n, LOW);
}
