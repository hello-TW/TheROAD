#define MAX_TRAP_SIZE 14
#define MIN_TRAP_SIZE 10

typedef struct _player_struct_ {
  int x;
  int y;
} player_t;

char mega1_pin[MAX_TRAP_SIZE] = {
    0,
};

char mega1_pin_size = 0;


const char pin_arr[4][8] = { // 4x8 mega1 and mega2 pin map
    {53, 45, 37, 29, 52, 44, 36, 28},
    {51, 43, 35, 27, 50, 42, 34, 26},
    {49, 41, 33, 25, 48, 40, 32, 24},
    {47, 39, 31, 23, 46, 38, 30, 22}};


const int trap_lookup[8][2] = {{1, 2}, {2, 1}, {-2, 1}, {-1, 2}, {2, -1}, {1, -2}, {-2, -1}, {-1, -2}};

void setup() {
  // put your setup code here, to run once:
  unsigned long interval;
  char trap[8][8] = {//3,5
  {'o','o','o','o','g','o','o','o'},
  {'o','o','o','x','x','x','x','x'},
  {'o','o','o','x','x','x','x','x'},
  {'o','o','o','x','x','o','x','x'},
  {'o','o','o','x','x','x','x','x'},
  {'o','o','o','x','x','x','x','x'},
  {'o','o','o','o','o','o','o','o'},
  {'1','o','o','o','o','o','o','2'},
  };
  player_t test = {3,5};
  Serial.begin(115200);
  Serial.println();
  ledAroundTrap(trap, test);
}

void loop() {
  // put your main code here, to run repeatedly:

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
  Serial.print("x : ");
  Serial.print(player.x);
  Serial.print("  y : ");
  Serial.println(player.y);
  Serial.println();
  mega1_pin_size = 0;
  
  for (i = -2; i <= 2; i++) {
    for (j = -2; j <= 2; j++) {
      
      if ((player.x + i < 0) || (player.x + i > 7) || (player.y + j < 0) || (player.y + j > 7)){
        Serial.println("con");
        continue;
      }
      
      if (trap[player.x + i][player.y + j] == 'x') {
        trap_pos[pos].x = player.x + i;
        trap_pos[pos].y = player.y + j;
        Serial.print("x : ");
        Serial.print(trap_pos[pos].x);
        Serial.print("  y : ");
        Serial.print(trap_pos[pos].y);
        
        Serial.print("  i : ");
        Serial.print(i);
        Serial.print("  j : ");
        Serial.println(j);
        pos++;
        delay(500);
      }
    }
  }
  Serial.println();
  for (i = 0; i < pos; i++) {
    Serial.print("x : ");
    Serial.print(trap_pos[i].x);
    Serial.print("  y : ");
    Serial.println(trap_pos[i].y);
    if (trap_pos[i].x > 3) {
      mega1_pin[mega1_pin_size] = pin_arr[trap_pos[i].x - 4][trap_pos[i].y];
      mega1_pin_size++;
      Serial.print("mega1 : ");
      Serial.println(pin_arr[trap_pos[i].x - 4][trap_pos[i].y],DEC);
    }

    else {
      mega2_pin[mega2_pin_size] = pin_arr[trap_pos[i].x][trap_pos[i].y];
      mega2_pin_size++;
      
      Serial.print("mega2 : ");
      Serial.println(pin_arr[trap_pos[i].x][trap_pos[i].y],DEC);
    }
    delay(500);
  }
  Serial.print("size1 : ");
  Serial.println((int)mega1_pin_size);
  Serial.print("size2 : ");
  Serial.println((int)mega2_pin_size);
}
