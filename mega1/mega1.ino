/**
 * Mega1.ino
 * 
 * Main code of 'The Road'
 * manages all communication and controls game flow.
 * 
 * @ Wifi communication protocol
 *    1:player
 *    2:command size
 *    3:command arrays or item code
 * 
 * @ Serial communication protocol
 *    1:size of pin-number
 *    2:pin-number arrays
 * 
*/

#include <LiquidCrystal_I2C.h>
#include <MsTimer2.h>
#include <SoftwareSerial.h>
#include <WiFiEsp.h>
#include <Wire.h>

#define RX 10 // mega2 - Serial Rx
#define TX 11 // mega2 - Serial Tx

#define SPEAKER1 5 //speaker1 pin
#define SPEAKER2 6 //speaker2 pin

#define M2_START 124  //mega2 - wifi initialize end and start welcome2 code
#define M2_END 125    //mega2 - game end code
#define M2_RETURN 126 //mega2 - return and restart code

#define BEE_TURN 125   //beeboy - arrive and change turn code
#define BEE_TRAPED 123 //beeboy - arrive trap pos code

#define MAX_TRAP_SIZE 14
#define MIN_TRAP_SIZE 10

#define MAX_GTRAP_SIZE 11
#define MIN_GTRAP_SIZE 8

#define NOTE_TIME 1000 //Notification time interval when trapped

typedef struct _player_struct_ {
  int x;
  int y;
} player_t;

/*
char ssid[] = "SSID";     // your network SSID (name)
char pass[] = "password"; // your network password
*/

char ssid[] = "Taewon";     // your network SSID (name)
char pass[] = "xodnjsdl98"; // your network password

/*

char ssid[] = "YG";     // your network SSID (name)
char pass[] = "99790676"; // your network password
*/
int status = WL_IDLE_STATUS;
char mega1_pin[MAX_TRAP_SIZE] = {
    0,
};

char mega1_pin_size = 0;

const int trap_lookup[8][2] = {{1, 2}, {2, 1}, {-2, 1}, {-1, 2}, {2, -1}, {1, -2}, {-2, -1}, {-1, -2}};

const int pin_arr[4][8] = { // 4x8 mega1 and mega2 pin map
    {53, 45, 37, 29, 52, 44, 36, 28},
    {51, 43, 35, 27, 50, 42, 34, 26},
    {49, 41, 33, 25, 48, 40, 32, 24},
    {47, 39, 31, 23, 46, 38, 30, 22}};

SoftwareSerial mySerial(RX, TX);
WiFiEspServer server(80);
LiquidCrystal_I2C lcd(0x27, 16, 2);

//flow function
char waitConnenct(void);
void sendConnectivity(int isSame);
void generalMode(void);
void itemMode(void);
void sendResult(char);
void calReturnCommand(char* command, player_t player, const player_t start_pos);
void HttpItem(int res, int traped_player, int attack_flag, int random_flag, WiFiEspClient* client);

//trap
void initTrap(char trap[][8]);
void makeTrap(char trap[][8]);
void showTrap(const char trap[][8], const player_t* players);
void calReturnCommand(char* command, player_t player, const player_t start_pos);
void calReturnCommandTurn(char* command, player_t player, const player_t start_pos);

//item
void random_attack(char* command, int com_size);

//led
void ledCallBack();
void ledPos(player_t player);
void ledAroundTrap(const char trap[][8], player_t player);
void ledAllTrap(const char trap[][8]);
void ledG(void);
void ledI(void);
void on(int pin);
void off(int pin);

void setup() { //initialize Serials, LCD, and Wifi
  for (int i = 22; i < 54; i++) {
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
  }
  Serial.begin(115200);
  Serial1.begin(115200);
  Serial2.begin(115200);
  Serial3.begin(115200);
  mySerial.begin(9600);
  welcome();

  lcd.init();
  lcd.backlight();

  randomSeed(analogRead(0));
  MsTimer2::set(NOTE_TIME, ledCallBack);

  lcd.setCursor(0, 0);
  lcd.print("POWER ON");

  WiFi.init(&Serial3); // initialize ESP module
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(ssid);
    status = WiFi.begin(ssid, pass);
  }

  lcd.clear();
  lcd.setCursor(0, 0);

  Serial.println("You're connected to the network");
  //printWifiStatus();
  server.begin();
}

void loop() {
  printWifiStatus();
  mySerial.write(M2_START); //end wifi initialize and transmit to mega2
  welcome2();
  char mode = waitConnect();
  if (mode == 'G'){
    ledG();
    generalMode();
  }
  else if (mode == 'I'){
    ledI();
    itemMode();
  }
  mode = 0;
  lcd.clear();
}

//flow
char waitConnect() { //Waiting for the connection to start
  int flag1 = 0, flag2 = 0, con_both = 0;
  char player1_mode = 0, player2_mode = 0, player_num = 0, mode = 0;

  lcd.setCursor(0, 0);
  lcd.print("Player");
  while (1) {
    WiFiEspClient client = server.available(); // listen for incoming clients
    if (client) {
      while (client.connected()) { // loop while the client's connected
        if (client.available()) {  // if there's bytes to read from the client,
          char c = client.read();
          if (c == '/') {
            player_num = client.read();
            mode = client.read();
            Serial.print("player : ");
            Serial.print(player_num);
            Serial.print("  mode : ");
            Serial.println(mode);
            break;
          }
        }
      }

      if (mode == 'G' || mode == 'I') {
        if (!flag1 && player_num == '1') {
          lcd.setCursor(6, 0);
          lcd.print("1: ");
          lcd.print(mode);
          flag1 = 1;
          player1_mode = mode;
          on(49); on(41); on(39);
          tone(SPEAKER1, 783, 250);
        }

        else if (!flag2 && player_num == '2') {
          lcd.setCursor(6, 0);
          lcd.print("2: ");
          lcd.print(mode);
          flag2 = 1;
          player2_mode = mode;
          on(30); on(32); on(24);
          tone(SPEAKER1, 783, 250);
        }
      }
      con_both = flag1 & flag2;
      Serial.print("flag1 : ");
      Serial.print(flag1);
      Serial.print("\tflag2 : ");
      Serial.println(flag2);

      Serial.print("mode1 : ");
      Serial.print(player1_mode);
      Serial.print("\tmode2 : ");
      Serial.println(player2_mode);

      client.flush();
      client.println("HTTP/1.1 200 OK");
      client.println();
      client.print("Player ");
      client.print(player_num);
      client.println(" is connected");
      client.stop();
    }
    if (con_both) {
      if (player1_mode == player2_mode) {
        Serial.println("\n");
        Serial.println("both connected");
        lcd.setCursor(0, 1);
        lcd.print("both connected! ");
        sendConnectivity(1);
        return player1_mode;
      }

      else { //If two-player connection as different mode, then it goes start
        Serial.print("differnt mode");
        lcd.setCursor(0, 1);
        lcd.print("differnt mode! ");
        sendConnectivity(2);
        flag1 = flag2 = 0;
        player_num = player1_mode = player2_mode = 0;
        off(49); off(41); off(39);
        off(30); off(32); off(24);
      }
    }
  }
}

void sendConnectivity(int isSame) {
  int send_flag1 = 0;
  int send_flag2 = 0;
  int send_both = 0;
  char player_num = 0;
  while (1) {
    WiFiEspClient client = server.available(); // listen for incoming clients
    if (client) {

      Serial.println("new client");
      while (client.connected()) { // loop while the client's connected
        if (client.available()) {  // if there's bytes to read from the client,
          char c = client.read();
          if (c == '/') {
            player_num = client.read();

            Serial.print("player : ");
            Serial.println(player_num);
            break;
          }
        }
      }

      if (!send_flag1 && player_num == '1') {
        send_flag1 = 1;
      } else if (!send_flag2 && player_num == '2') {
        send_flag2 = 1;
      }

      send_both = send_flag1 & send_flag2;

      Serial.print("send_flag1 : ");
      Serial.print(send_flag1);
      Serial.print("   send_flag2 : ");
      Serial.println(send_flag2);

      client.flush();
      client.println("HTTP/1.1 200 OK");
      client.println();
      client.print("{\"connection\":");
      client.print(isSame);
      client.println("}");
      client.println();
      client.stop();

      if (send_both){
        off(49); off(41); off(39);
        off(30); off(32); off(24);
        return;
      }
    }
  }
}

void generalMode() {

  char trap[8][8] = {
      0,
  };

  char command[10] = {
      //command for receiveing from wifi
      0,
  };
  char beeCommand[10] = {
      //command for transmission
      0,
  };
  char reCommand[18] = {
      //command for goning to start-pos
      0,
  };
  char player_num = 0;

  int command_size = 0;
  int turn = 0; // 0: player1   1: player2
  int res = 0;  // 1: ok        2:wrong turn
  int winner = 0;
  int traped_player = 0;
  int i, j;
  int command_flag = 1; //to avoid receiving twice
  int beepos = 0;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Generating map..");
  const player_t start_pos[2] = {{7, 0}, {7, 7}};
  player_t player[2] = {{7, 0}, {7, 7}};

  makeGTrap(trap);
  showTrap(trap, player);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Player 1 turn");
  lcd.setCursor(0, 1);
  lcd.print("Command:"); //8
  while (1) {
    WiFiEspClient client = server.available();
    if (client) {
      Serial.println("new client");
      Serial.println(command_flag);
      if (command_flag) {
        while (client.connected()) {
          if (client.available()) {
            char c = client.read();
            if (c == '/') {
              res = 1;
              player_num = client.read();
              Serial.print("player : ");
              Serial.print(player_num);

              command_size = client.read() - 48;
              Serial.print("  command_size : ");
              Serial.print(command_size);

              for (i = 0; i < command_size; i++) {
                command[i] = client.read();
              }
              command[i] = '\0';

              Serial.print("  command : ");
              Serial.println(command);
              break;
            }
          }
        }

        lcd.setCursor(8, 1);

        if (turn != player_num - 49) {
          res = 2;
          Serial.println("wrong turn");
        }

        else if (command_size > 0) {
          beepos = 0;
          for (i = 0; i < 8; i++) {
            beeCommand[i] = 0;
          }
          lcd.print(command);
          Serial.println("Right turn");
          for (i = 0; i < command_size; i++) { //check command
            switch (command[i]) {
            case 'U':
              player[turn].x = player[turn].x - 1;
              break;
            case 'D':
              player[turn].x = player[turn].x + 1;
              break;
            case 'R':
              player[turn].y = player[turn].y + 1;
              break;
            case 'L':
              player[turn].y = player[turn].y - 1;
              break;
            default:
              Serial.println("Wrong command!");
              break;
            }
            if (player[turn].x < 0) {
              player[turn].x = 0;
              continue;
            } else if (player[turn].x > 7) {
              player[turn].x = 7;
              continue;
            } else if (player[turn].y < 0) {
              player[turn].y = 0;
              continue;
            } else if (player[turn].y > 7) {
              player[turn].y = 7;
              continue;
            } else {
              beeCommand[beepos++] = command[i];
            }
            if (trap[player[turn].x][player[turn].y] == 'x') { // traped
              Serial.print("player ");
              Serial.print(turn + 1);
              Serial.println("will be traped");
              beeCommand[beepos++] = 't';
              break;
            } else if (trap[player[turn].x][player[turn].y] == 'g') { //goal
              Serial.print("player ");
              Serial.print(turn + 1);
              Serial.println("will win");
              winner = turn + 1;
              beeCommand[beepos++] = 'e';
              break;
            } else if (trap[player[turn].x][player[turn].y] == (turn + 1) % 2 + 49) {
              Serial.print("player ");
              Serial.print(turn + 1);
              Serial.println("will crush");
              beepos++;
              break;
            }
          } //for end
          if(beepos == 0){
            beeCommand[0] = 'n';
            beepos++;
          }
          beeCommand[beepos] = '\0';

          Serial.print("command to player ");
          Serial.print(turn + 1);
          Serial.print(" ");
          Serial.println(beeCommand);

          Serial.print("pos : ");
          Serial.print(player[turn].x);
          Serial.print(", ");
          Serial.println(player[turn].y);

          if (turn == 0) {
            Serial1.print(beeCommand);
            command_flag = 0;
          }

          else if (turn == 1) {
            Serial2.print(beeCommand);
            command_flag = 0;
          }
        }
      }
      client.flush();
      client.println("HTTP/1.1 200 OK");
      client.println();
      client.print("{\"res\":");
      client.print(res);
      client.print(", \"trap\":");
      client.print(traped_player);
      client.println("}");
      client.stop();

      if (turn + 1 == player_num - 48)
        traped_player = 0;
      Serial.println();
      Serial.print("turn : ");
      Serial.println(turn);

      showTrap(trap, player);
    }

    if (winner) {
      sendResult(winner);

      calReturnCommand(reCommand, player[0], start_pos[0]);
      Serial1.print(reCommand);
      calReturnCommand(reCommand, player[1], start_pos[1]);
      Serial2.print(reCommand);
      //led on
      ledAllTrap(trap);
      return;
    }

    if (Serial1.available()) {
      char read1 = Serial1.read();
      Serial.println();
      Serial.print("Serial1 read");
      Serial.println((int)read1);
      if (read1 == BEE_TURN) {
        turn = (turn + 1) % 2;
        command_flag = 1;
        lcd.setCursor(7, 0);
        lcd.print(turn + 1);
        lcd.setCursor(8, 1);
        lcd.print("   ");
      }

      else if (read1 == BEE_TRAPED) {
        traped_player = 1;
        ledPos(player[0]);
        tone(SPEAKER1, 440, NOTE_TIME);
        tone(SPEAKER2, 440, NOTE_TIME);
        calReturnCommandTurn(reCommand, player[0], start_pos[0]);
        player[0].x = start_pos[0].x;
        player[0].y = start_pos[0].y;
        Serial.print("reCommand : ");
        Serial.println(reCommand);
        Serial1.print(reCommand);
      }
    }
    if (Serial2.available()) {
      char read2 = Serial2.read();
      Serial.println();
      Serial.print("Serial2 read");
      Serial.println((int)read2);
      if (read2 == BEE_TURN) {
        turn = (turn + 1) % 2;
        command_flag = 1;
        lcd.setCursor(7, 0);
        lcd.print(turn + 1);
        lcd.setCursor(8, 1);
        lcd.print("   ");
      }

      else if (read2 == BEE_TRAPED) {
        traped_player = 2;
        ledPos(player[1]);
        tone(SPEAKER1, 440, NOTE_TIME);
        tone(SPEAKER2, 440, NOTE_TIME);
        calReturnCommandTurn(reCommand, player[1], start_pos[1]);
        player[1].x = start_pos[1].x;
        player[1].y = start_pos[1].y;
        Serial.print("reCommand : ");
        Serial.println(reCommand);
        Serial2.print(reCommand);
        // led on
      }
    }
  }
}

void itemMode() {
  
  char trap[8][8] = {
      0,
  };
  
  char command[7] = {
      0,
  };
  char beeCommand[8] = {
      0,
  };
  char reCommand[18] = {
      0,
  };
  char player_num = 0;

  int command_size = 0, turn = 0, winner = 0;

  int res = 0; // 1: ok    2:wrong turn    3: traped

  int attack_flag = 0, barrior_flag = 0, random_flag = 0, traped_player = 0; // json flag

  int random_turn_flag = 0;
  int command_flag = 1;
  int i, j, bee_pos;
  bool isInvincibility = 0; // 1: ignor traps

  const player_t start_pos[2] = {{7, 0}, {7, 7}};
  player_t player[2] = {{7, 0}, {7, 7}};

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Generating map..");
  
  makeTrap(trap);
  showTrap(trap, player);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Player 1 turn");
  lcd.setCursor(0, 1);
  lcd.print("Command:"); //8

  while (1) {
    WiFiEspClient client = server.available();
    if (client) {
      Serial.println("new client");
      if (command_flag) {
        while (client.connected()) {
          if (client.available()) {
            char c = client.read();
            if (c == '/') {
              res = 1;
              player_num = client.read();
              Serial.print("player : ");
              Serial.print(player_num);
              if (player_num != '1' && player_num != '2') {
                break;
              }
              command_size = client.read() - 48;
              Serial.print("  command_size : ");
              Serial.print(command_size);

              for (i = 0; i < command_size; i++) {
                command[i] = client.read();
              }
              command[i] = '\0';

              Serial.print("  command : ");
              Serial.println(command);
              break;
            }
          }
        }
        lcd.setCursor(8, 1);
        if (turn != player_num - 49) {
          res = 2;
          Serial.println("wrong turn");
        }

        else if (command_size == 1) { //item is entered
          Serial.println(command[0]);
          lcd.print(command[0]);
          switch (command[0]) {
          case 's':
            ledAroundTrap(trap, player[turn]);
            break;
          case 'r':
            random_flag = (turn + 1) % 2 + 1;
            random_turn_flag = random_flag;
            turn = (turn + 1) % 2;
            break;
          case 'a': //attact another player
            attack_flag = (turn + 1) % 2 + 1;
            turn = (turn + 1) % 2;
            break;
          case 'b':
            isInvincibility = 1;
            break;
          case 'i':
            if (attack_flag)
              attack_flag = 0;
            if (random_turn_flag){
              random_flag = 0;
              random_turn_flag = 0;
            }
            turn = (turn + 1) % 2;
            break;
          case 'n':
            if (attack_flag) {
              calReturnCommandTurn(reCommand, player[turn], start_pos[turn]);
              if (turn == 0)
                Serial1.print(reCommand);
              else if (turn == 1)
                Serial2.print(reCommand);
              player[turn].x = start_pos[turn].x;
              player[turn].y = start_pos[turn].y;
              attack_flag = 0;
            }
            if (random_flag) {
              random_flag = 0;
              turn = (turn + 1) % 2;
            }
            break;
          }
        }

        else if (command_size > 1) {
          bee_pos = 0;
          for (i = 0; i < 8; i++) {
            beeCommand[i] = 0;
          }
          Serial.println("Right turn");
          if (random_turn_flag == player_num - 48) {
            Serial.println("changing command");
            random_attack(command, command_size);
            random_turn_flag = 0;
          }

          lcd.print(command);

          for (i = 0; i < command_size; i++) {
            switch (command[i]) {
            case 'U':
              player[turn].x = player[turn].x - 1;
              break;
            case 'D':
              player[turn].x = player[turn].x + 1;
              break;
            case 'R':
              player[turn].y = player[turn].y + 1;
              break;
            case 'L':
              player[turn].y = player[turn].y - 1;
              break;
            }
            if (player[turn].x < 0) {
              player[turn].x = 0;
              continue;
            } else if (player[turn].x > 7) {
              player[turn].x = 7;
              continue;
            } else if (player[turn].y < 0) {
              player[turn].y = 0;
              continue;
            } else if (player[turn].y > 7) {
              player[turn].y = 7;
              continue;
            } else {
              beeCommand[bee_pos++] = command[i];
            }
            if (trap[player[turn].x][player[turn].y] == 'x') { // traped
              if(isInvincibility){
                ledPos(player[turn]);
                continue;
              }
              Serial.print("player ");
              Serial.print(turn + 1);
              Serial.println("will be traped");
              beeCommand[bee_pos++] = 't';
              break;
            } else if (trap[player[turn].x][player[turn].y] == 'g') { //goal
              Serial.print("player ");
              Serial.print(turn + 1);
              Serial.println("will win");
              winner = turn + 1;
              beeCommand[bee_pos++] = 'e';
              break;
            }

            else if (trap[player[turn].x][player[turn].y] == (turn + 1) % 2 + 49) {
              Serial.print("player ");
              Serial.print(turn + 1);
              Serial.println("will crush");
              bee_pos++;
              break;
            }
          }
          if(bee_pos == 0){
            beeCommand[0] = 'n';
            bee_pos++;
          }
          beeCommand[bee_pos] = '\0';

          Serial.print("command to player ");
          Serial.print(turn + 1);
          Serial.print(" ");
          Serial.println(beeCommand);

          Serial.print("pos : ");
          Serial.print(player[turn].x);
          Serial.print(", ");
          Serial.println(player[turn].y);
          isInvincibility = 0;

          if (turn == 0) { //player1 turn
            Serial1.print(beeCommand);
            //turn = (turn+1)%2;
            command_flag = 0;
          }

          else if (turn == 1) {
            Serial2.print(beeCommand);
            //turn = (turn+1)%2;
            command_flag = 0;
          }
        }
      }

      HttpItem(res, traped_player, attack_flag, random_flag, &client);

      if (turn + 1 == player_num - 48) {
        traped_player = 0;
      }
      Serial.println();
      Serial.print("turn : ");
      Serial.print(turn);
      Serial.print("  Attack : ");
      Serial.print(attack_flag);
      Serial.print("  Rand : ");
      Serial.print(random_flag);

      Serial.println('\n');
      showTrap(trap, player);
    }

    if (winner) {
      sendResult(winner);
      calReturnCommand(reCommand, player[0], start_pos[0]);
      Serial1.print(reCommand);
      calReturnCommand(reCommand, player[1], start_pos[1]);
      Serial2.print(reCommand);
      ledAllTrap(trap);
      //trap led on
      return;
    }

    if (Serial1.available()) {
      char read1 = Serial1.read();
      Serial.println();
      Serial.print("Serial1 read");
      Serial.println((int)read1);
      if (read1 == BEE_TURN) {
        turn = (turn + 1) % 2;
        command_flag = 1;
        lcd.setCursor(7, 0);
        lcd.print(turn + 1);
        lcd.setCursor(8, 1);
        lcd.print("      ");
      }

      else if (read1 == BEE_TRAPED) {
        traped_player = 1;
        ledPos(player[0]);
        tone(SPEAKER1, 440, NOTE_TIME);
        tone(SPEAKER2, 440, NOTE_TIME);
        calReturnCommandTurn(reCommand, player[0], start_pos[0]);
        player[0].x = start_pos[0].x;
        player[0].y = start_pos[0].y;
        Serial.print("reCommand : ");
        Serial.println(reCommand);
        Serial1.print(reCommand);
      }
    }
    if (Serial2.available()) {
      char read2 = Serial2.read();
      Serial.println();
      Serial.print("Serial2 read");
      Serial.println((int)read2);
      if (read2 == BEE_TURN) {
        turn = (turn + 1) % 2;
        command_flag = 1;
        lcd.setCursor(7, 0);
        lcd.print(turn + 1);
        lcd.setCursor(8, 1);
        lcd.print("      ");
      }

      else if (read2 == BEE_TRAPED) {
        traped_player = 2;
        ledPos(player[1]);
        tone(SPEAKER1, 440, NOTE_TIME);
        tone(SPEAKER2, 440, NOTE_TIME);
        calReturnCommandTurn(reCommand, player[1], start_pos[1]);
        player[1].x = start_pos[1].x;
        player[1].y = start_pos[1].y;
        Serial.print("reCommand : ");
        Serial.println(reCommand);
        Serial2.print(reCommand);
        // led on
      }
    }
  }
}

void sendResult(int winner) { //send winner
  Serial.println("start sendwinner");
  int send_flag1 = 0, send_flag2 = 0, send_both = 0, command_size = 0;
  char player_num = 0;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Player");
  lcd.setCursor(6, 0);
  lcd.print(winner);
  lcd.setCursor(8, 0);
  lcd.print("is win");

  while (1) {
    WiFiEspClient client = server.available(); // listen for incoming clients
    if (client) {
      Serial.println("new client");
      while (client.connected()) { // loop while the client's connected
        if (client.available()) {  // if there's bytes to read from the client,
          char c = client.read();
          if (c == '/') {
            player_num = client.read();
            Serial.print("player : ");
            Serial.println(player_num);
            break;
          }
        }
      }

      if (!send_flag1 && player_num == '1') {
        send_flag1 = 1;

      } else if (!send_flag2 && player_num == '2') {
        send_flag2 = 1;
      }

      send_both = send_flag1 & send_flag2;

      Serial.print("send_flag1 : ");
      Serial.print(send_flag1);
      Serial.print("\tsend_flag2 : ");
      Serial.println(send_flag2);

      client.flush();
      client.println("HTTP/1.1 200 OK");
      client.println();
      client.print("{\"winner\":");
      client.print(winner);
      client.println("}");
      client.println();
      client.stop();
    }
    if (send_both)
      return;
  }
}

//wifi
void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  Serial.println();
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
  Serial.println();

  lcd.setCursor(0, 1);
  lcd.print(ip);
}

void HttpItem(int res, int traped_player, int attack_flag, int random_flag, WiFiEspClient* client) {

  client->flush();
  client->println("HTTP/1.1 200 OK");
  client->println();
  client->print("{\"res\":");
  client->print(res);
  client->print(", \"trap\":");
  client->print(traped_player);
  client->print(", \"attack\":");
  client->print(attack_flag);
  client->print(", \"rand\":");
  client->print(random_flag);
  client->println("}");
  client->stop();
}

//trap
void initTrap(char trap[][8]) {
  int i, j;
  for (i = 0; i < 8; i++)
    for (j = 0; j < 8; j++)
      trap[i][j] = 'o';

  trap[7][0] = '1'; //player 1
  trap[7][7] = '2'; //player 2
  trap[0][4] = 'g'; //goal
}

void makeTrap(char trap[][8]) {

  int rand_x, rand_y;
  int i, j;
  int trap_num, pin_pos;
  int left_num, right_num;
  while (1) {
    trap_num = 0;
    pin_pos = 0;
    left_num = 0;
    right_num = 0;
    initTrap(trap);
    while (1) {
      rand_x = random(0, 3);
      rand_y = random(0, 3);
      if (trap[rand_x][rand_y] == 'o' && rand_x != 2 && rand_y != 2) {
        trap[rand_x][rand_y] = 'x';
        trap_num++;
        break;
      }
    }

    for (i = 0; i < 8; i++) {
      if ((rand_x + trap_lookup[i][0] < 0) ||
          (rand_x + trap_lookup[i][0] > 7) ||
          (rand_y + trap_lookup[i][1] < 0) ||
          (rand_y + trap_lookup[i][1] > 7))
        continue;
      if ((random(0, 3) == 0) && (trap[rand_x + trap_lookup[i][0]][rand_y + trap_lookup[i][1]] == 'o')) {
        trap[rand_x + trap_lookup[i][0]][rand_y + trap_lookup[i][1]] = 'x';
        trap_num++;
      }
    }

    //2.
    while (1) {
      rand_x = random(4, 7);
      rand_y = random(0, 3);
      if (trap[rand_x][rand_y] == 'o' && rand_x != 2 && rand_y != 6) {
        trap[rand_x][rand_y] = 'x';
        trap_num++;
        break;
      }
    }

    for (i = 0; i < 8; i++) {
      if ((rand_x + trap_lookup[i][0] < 0) ||
          (rand_x + trap_lookup[i][0] > 7) ||
          (rand_y + trap_lookup[i][1] < 0) ||
          (rand_y + trap_lookup[i][1] > 7))
        continue;
      if ((random(0, 3) == 0) && (trap[rand_x + trap_lookup[i][0]][rand_y + trap_lookup[i][1]] == 'o')) {
        trap[rand_x + trap_lookup[i][0]][rand_y + trap_lookup[i][1]] = 'x';
        trap_num++;
      }
    }
    //3.
    while (1) {
      rand_x = random(0, 3);
      rand_y = random(4, 7);
      if (trap[rand_x][rand_y] == 'o' && rand_x != 2 && rand_y != 5) {
        trap[rand_x][rand_y] = 'x';
        trap_num++;
        break;
      }
    }

    for (i = 0; i < 8; i++) {
      if ((rand_x + trap_lookup[i][0] < 0) ||
          (rand_x + trap_lookup[i][0] > 7) ||
          (rand_y + trap_lookup[i][1] < 0) ||
          (rand_y + trap_lookup[i][1] > 7))
        continue;
      if ((random(0, 3) == 0) && (trap[rand_x + trap_lookup[i][0]][rand_y + trap_lookup[i][1]] == 'o')) {
        trap[rand_x + trap_lookup[i][0]][rand_y + trap_lookup[i][1]] = 'x';
        trap_num++;
      }
    }

    //4.
    while (1) {
      rand_x = random(4, 7);
      rand_y = random(4, 7);
      if (trap[rand_x][rand_y] == 'o' && rand_x != 5 && rand_y != 5) {
        trap[rand_x][rand_y] = 'x';
        trap_num++;
        break;
      }
    }

    for (i = 0; i < 8; i++) {
      if ((rand_x + trap_lookup[i][0] < 0) ||
          (rand_x + trap_lookup[i][0] > 7) ||
          (rand_y + trap_lookup[i][1] < 0) ||
          (rand_y + trap_lookup[i][1] > 7))
        continue;
      if ((random(0, 3) == 0) && (trap[rand_x + trap_lookup[i][0]][rand_y + trap_lookup[i][1]] == 'o')) {
        trap[rand_x + trap_lookup[i][0]][rand_y + trap_lookup[i][1]] = 'x';
        trap_num++;
      }
    }
    for (i = 0; i < 4; i++)
      for (j = 0; j < 8; j++)
        if (trap[j][i] == 'x')
          left_num++;
    for (i = 4; i < 8; i++)
      for (j = 0; j < 8; j++)
        if (trap[j][i] == 'x')
          right_num++;

    if (left_num != right_num)
      continue;

    if (trap_num >= MIN_TRAP_SIZE && trap_num <= MAX_TRAP_SIZE) {
      Serial.print("\nTrap Num : ");
      Serial.println(trap_num);
      break;
    }
  }
}

void makeGTrap(char trap[][8]) {

  int rand_x, rand_y;
  int i, j;
  int trap_num, pin_pos;
  int left_num, right_num;
  while (1) {
    trap_num = 0;
    pin_pos = 0;
    left_num = 0;
    right_num = 0;
    initTrap(trap);
    while (1) {
      rand_x = random(0, 3);
      rand_y = random(0, 3);
      if (trap[rand_x][rand_y] == 'o' && rand_x != 2 && rand_y != 2) {
        trap[rand_x][rand_y] = 'x';
        trap_num++;
        break;
      }
    }

    for (i = 0; i < 8; i++) {
      if ((rand_x + trap_lookup[i][0] < 0) ||
          (rand_x + trap_lookup[i][0] > 7) ||
          (rand_y + trap_lookup[i][1] < 0) ||
          (rand_y + trap_lookup[i][1] > 7))
        continue;
      if ((random(0, 4) == 0) && (trap[rand_x + trap_lookup[i][0]][rand_y + trap_lookup[i][1]] == 'o')) {
        trap[rand_x + trap_lookup[i][0]][rand_y + trap_lookup[i][1]] = 'x';
        trap_num++;
      }
    }

    //2.
    while (1) {
      rand_x = random(4, 7);
      rand_y = random(0, 3);
      if (trap[rand_x][rand_y] == 'o' && rand_x != 2 && rand_y != 6) {
        trap[rand_x][rand_y] = 'x';
        trap_num++;
        break;
      }
    }

    for (i = 0; i < 8; i++) {
      if ((rand_x + trap_lookup[i][0] < 0) ||
          (rand_x + trap_lookup[i][0] > 7) ||
          (rand_y + trap_lookup[i][1] < 0) ||
          (rand_y + trap_lookup[i][1] > 7))
        continue;
      if ((random(0, 4) == 0) && (trap[rand_x + trap_lookup[i][0]][rand_y + trap_lookup[i][1]] == 'o')) {
        trap[rand_x + trap_lookup[i][0]][rand_y + trap_lookup[i][1]] = 'x';
        trap_num++;
      }
    }
    //3.
    while (1) {
      rand_x = random(0, 3);
      rand_y = random(4, 7);
      if (trap[rand_x][rand_y] == 'o' && rand_x != 2 && rand_y != 5) {
        trap[rand_x][rand_y] = 'x';
        trap_num++;
        break;
      }
    }

    for (i = 0; i < 8; i++) {
      if ((rand_x + trap_lookup[i][0] < 0) ||
          (rand_x + trap_lookup[i][0] > 7) ||
          (rand_y + trap_lookup[i][1] < 0) ||
          (rand_y + trap_lookup[i][1] > 7))
        continue;
      if ((random(0, 4) == 0) && (trap[rand_x + trap_lookup[i][0]][rand_y + trap_lookup[i][1]] == 'o')) {
        trap[rand_x + trap_lookup[i][0]][rand_y + trap_lookup[i][1]] = 'x';
        trap_num++;
      }
    }

    //4.
    while (1) {
      rand_x = random(4, 7);
      rand_y = random(4, 7);
      if (trap[rand_x][rand_y] == 'o' && rand_x != 5 && rand_y != 5) {
        trap[rand_x][rand_y] = 'x';
        trap_num++;
        break;
      }
    }

    for (i = 0; i < 8; i++) {
      if ((rand_x + trap_lookup[i][0] < 0) ||
          (rand_x + trap_lookup[i][0] > 7) ||
          (rand_y + trap_lookup[i][1] < 0) ||
          (rand_y + trap_lookup[i][1] > 7))
        continue;
      if ((random(0, 4) == 0) && (trap[rand_x + trap_lookup[i][0]][rand_y + trap_lookup[i][1]] == 'o')) {
        trap[rand_x + trap_lookup[i][0]][rand_y + trap_lookup[i][1]] = 'x';
        trap_num++;
      }
    }
    for (i = 0; i < 4; i++)
      for (j = 0; j < 8; j++)
        if (trap[j][i] == 'x')
          left_num++;
    for (i = 4; i < 8; i++)
      for (j = 0; j < 8; j++)
        if (trap[j][i] == 'x')
          right_num++;

    if (left_num != right_num)
      continue;

    if (trap_num >= MIN_GTRAP_SIZE && trap_num <= MAX_GTRAP_SIZE) {
      Serial.print("\nTrap Num : ");
      Serial.println(trap_num);
      break;
    }
  }
}

void showTrap(const char trap[][8], const player_t* players) {
  int i, j;
  for (i = 0; i < 8; i++) {
    for (j = 0; j < 8; j++) {
      if (players[0].x == i && players[0].y == j) {
        Serial.print(1);
        Serial.print(" ");
      } else if (players[1].x == i && players[1].y == j) {
        Serial.print(2);
        Serial.print(" ");
      }

      else {
        Serial.print(trap[i][j]);
        Serial.print(" ");
      }
    }
    Serial.println();
  }
}

void calReturnCommandTurn(char* command, player_t player, const player_t start_pos) {
  int difx = start_pos.x - player.x;
  int dify = start_pos.y - player.y;
  int i;
  int index = 0;

  if (dify < 0) {
    dify = dify * (-1);
    for (i = 0; i < dify; i++) {
      command[index++] = 'L';
    }
  }

  else {
    for (i = 0; i < dify; i++) {
      command[index++] = 'R';
    }
  }

  for (i = 0; i < difx; i++) {
    command[index++] = 'D';
  }

  
  command[index] = 'r';
  index++;
  command[index] = '\0';
}

void calReturnCommand(char* command, player_t player, const player_t start_pos) {
  int difx = start_pos.x - player.x;
  int dify = start_pos.y - player.y;
  int i;
  int index = 0;
  if (dify < 0) {
    dify = dify * (-1);
    for (i = 0; i < dify; i++) {
      command[index++] = 'L';
    }
  }

  else {
    for (i = 0; i < dify; i++) {
      command[index++] = 'R';
    }
  }
  for (i = 0; i < difx; i++) {
    command[index++] = 'D';
  }

  
  command[index] = 'e';
  index++;
  command[index] = '\0';
}

//item
void random_attack(char* command, int com_size) {
  const char c[4] = {'U', 'R', 'D', 'L'};
  int i;
  for (i = 0; i < com_size; i++) {
    command[i] = c[random(0, 3)];
  }
  command[i] = '\0';
  Serial.print("RANDOM : ");
  Serial.print(command);
  Serial.print("  I : ");
  Serial.println(i);
}

//timer
void ledCallBack() {
  int i;
  for (i = 0; i < mega1_pin_size; i++) {
    off(mega1_pin[i]);
    Serial.print("led off : ");
    Serial.println((int)mega1_pin[i]);
  }
  MsTimer2::stop();
}

//led
void ledPos(player_t player) {

  char pin[2] = {1, 0};

  if (player.x > 3) { // MEGA1 제어

    mega1_pin[0] = pin_arr[player.x - 4][player.y];
    mega1_pin_size = 1;

    on(mega1_pin[0]);

    Serial.print("led on : ");
    Serial.println((int)mega1_pin[0]);
    MsTimer2::start();
  } else { //mega2
    pin[1] = pin_arr[player.x][player.y];
    mySerial.write(pin, 2);
  }
}

void ledAroundTrap(const char trap[][8], player_t player) {

  player_t trap_pos[MAX_TRAP_SIZE] = {
      0,
  };

  char mega2_pin[MAX_TRAP_SIZE + 1] = {
      0,
  };
  char mega2_pin_size = 0;

  int i, j;
  int pos = 0;

  mega1_pin_size = 0;

  for (i = -2; i < 3; i++) {
    for (j = -2; j < 3; j++) {
      if ((player.x + i < 0) || (player.x + i > 7) || (player.y + j < 0) || (player.y + j > 7)) {
        continue;
      }
      if (trap[player.x + i][player.y + j] == 'x') {
        trap_pos[pos].x = player.x + i;
        trap_pos[pos].y = player.y + j;
        pos++;
      }
    }
  }

  Serial.print("pos : ");
  Serial.print(pos);
  Serial.print("  x: ");
  Serial.print(trap_pos[i].x);

  Serial.print("  y: ");
  Serial.println(trap_pos[i].y);
  for (i = 0; i < pos; i++) {
    if (trap_pos[i].x > 3) {
      mega1_pin[mega1_pin_size] = pin_arr[trap_pos[i].x - 4][trap_pos[i].y];
      mega1_pin_size++;
    }

    else {
      mega2_pin[mega2_pin_size + 1] = pin_arr[trap_pos[i].x][trap_pos[i].y];
      mega2_pin_size++;
    }
  }

  mega2_pin[0] = mega2_pin_size;

  Serial.print("led on : ");
  for (i = 0; i < mega1_pin_size; i++) {
    on(mega1_pin[i]);
    Serial.print((int)mega1_pin[i]);
    Serial.print(", ");
  }

  for (i = 1; i < mega2_pin_size+1; i++) {
    Serial.print((int)mega2_pin[i]);
    Serial.print(", ");
  }
  Serial.println();
  MsTimer2::start();

  mySerial.write(mega2_pin, mega2_pin[0] + 1);
}

void ledAllTrap(const char trap[][8]) {
  int i, j;
  char mega1_trap_pin[MAX_TRAP_SIZE] = {
      0,
  };
  char mega2_trap_pin[MAX_TRAP_SIZE] = {
      0,
  };
  int pos1 = 0, pos2 = 0;
  Serial.print("all trap in mega2 led on :");
  for (i = 0; i < 4; i++) {
    for (j = 0; j < 8; j++) {
      if (trap[i][j] == 'x') {
        mega2_trap_pin[pos2++] = pin_arr[i][j];
        Serial.print(pin_arr[i][j]);
        Serial.print(", ");
      }
    }
  }
  Serial.print("\nall trap in mega1 led on :");
  for (; i < 8; i++) {
    for (j = 0; j < 8; j++) {
      if (trap[i][j] == 'x') {
        mega1_trap_pin[pos1++] = pin_arr[i - 4][j];
        Serial.print(pin_arr[i - 4][j]);
        Serial.print(", ");
      }
    }
  }
  mySerial.write(M2_END);
  for (i = 0; i < pos2; i++) {
    mySerial.write(mega2_trap_pin[i]);
    delay(400);
  }
  for (i = 0; i < pos1; i++) {
    on(mega1_trap_pin[i]);
    delay(400);
  }
  delay(10000);
  mySerial.write(M2_RETURN);
  for (i = 0; i < pos1; i++) {
    digitalWrite(mega1_trap_pin[i], LOW);
  }
  delay(1000);
}
void ledG(void){
  char mega2_pin[9] = {
      8,35,27,50,42,33,31,46,38
  };
  on(37); on(44); on(35); on(27); on(50); on(42);
  mega1_pin[0] = 37; mega1_pin[1] = 44;
  mega1_pin[2] = 35; mega1_pin[3] = 27;
  mega1_pin[4] = 50; mega1_pin[5] = 42;
  mega1_pin_size = 6;
  MsTimer2::start();
  mySerial.write(mega2_pin, 9);
  
  return;
}
void ledI(void){
  char mega2_pin[9] = {
      8,35,27,50,42,25,48,23,46
  };
  on(29); on(52); on(35); on(27); on(50); on(42);
  mega1_pin[0] = 29; mega1_pin[1] = 52;
  mega1_pin[2] = 35; mega1_pin[3] = 27;
  mega1_pin[4] = 50; mega1_pin[5] = 42;
  mega1_pin_size = 6;
  MsTimer2::start();
  mySerial.write(mega2_pin, 9);
  return;
}

void welcome(){

  delay(400); 
  on(53);delay(360);
  on(45);delay(40);on(51);delay(40);
  on(49);delay(40);on(43);delay(40);on(37);delay(360);
  on(29);delay(40);on(35);delay(40);on(41);delay(40);on(47);delay(80);
  on(39);delay(40);on(33);delay(40);on(27);delay(40);on(52);delay(240);
  on(44);delay(40);on(50);delay(40);on(25);delay(40);on(31);delay(80);
  on(23);delay(40);on(48);delay(40);on(42);delay(40);on(36);delay(80);
  on(28);delay(40);on(34);delay(40);on(40);delay(40);on(46);delay(40);
  on(38);delay(40);on(32);delay(40);on(26);delay(40);
  on(24);delay(40);on(30);delay(40);
  on(22);
  delay(100);

  off(22);off(24);off(26);off(28);delay(100);
  off(30);off(32);off(34);off(36);delay(100);
  off(38);off(40);off(42);off(44);delay(100);
  off(46);off(48);off(50);off(52);delay(100);
  off(23);off(25);off(27);off(29);delay(100);
  off(31);off(33);off(35);off(37);delay(100);
  off(39);off(41);off(43);off(45);delay(100);  
  off(47);off(49);off(51);off(53);delay(100); 
  delay(100); //////////////////////////////////////////////////

  
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
  //////////////////////////////////////////////////////
}
void welcome2(){
  delay(50);
  on(29);on(52);delay(200);
  off(29);off(52);
  on(37);on(35);on(27);on(50);on(42);on(44);delay(200);
  off(37);off(35);off(27);off(50);off(42);off(44);
  on(45);on(43);on(41);on(33);on(25);on(48);on(40);on(32);on(34);on(36);delay(200);
  off(45);off(43);off(41);off(33);off(25);off(48);off(40);off(32);off(34);off(36);
  on(53);on(51);on(49);on(47);on(39);on(31);on(23);on(46);on(38);on(30);on(22);on(24);on(26);on(28);delay(200);
  off(53);off(51);off(49);off(47);off(39);off(31);off(23);off(46);off(38);off(30);off(22);off(24);off(26);off(28);
  on(45);on(43);on(41);on(33);on(25);on(48);on(40);on(32);on(34);on(36);delay(200);
  off(45);off(43);off(41);off(33);off(25);off(48);off(40);off(32);off(34);off(36);
  on(37);on(35);on(27);on(50);on(42);on(44);delay(200);
  off(37);off(35);off(27);off(50);off(42);off(44);
  on(29);on(52);delay(200);
  off(29);off(52);
}


void on(int pin_n) {
  digitalWrite(pin_n, HIGH);
}

void off(int pin_n) {
  digitalWrite(pin_n, LOW);
}
