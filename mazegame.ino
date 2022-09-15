/**
   Mark Zhou
   Mazegame
   9.14.22
   Intensive Physical Computing + Robotics Arr. 4
**/

// include the library code:
#include <LiquidCrystal.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 7, en = 8, d4 = 9, d5 = 10, d6 = 11, d7 = 12;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

byte pieces[8][8] = {
  {
    B11111,
    B10000,
    B10000,
    B10000,
    B10000,
    B10000,
    B10000,
    B10000
  },
  {
    B11111,
    B00001,
    B00001,
    B00001,
    B00001,
    B00001,
    B00001,
    B00001
  },
  {
    B11111,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B11111
  },
  {
    B10000,
    B10000,
    B10000,
    B10000,
    B10000,
    B10000,
    B10000,
    B11111
  },
  {
    B00001,
    B00001,
    B00001,
    B00001,
    B00001,
    B00001,
    B00001,
    B11111
  },
  {
    B11111,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000
  },
  {
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B11111
  },
  {
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000
  }
};

void setup() {
  // set up buttons
  for (int i = 14; i <= 15; i++) {
    pinMode(i, INPUT);
  }
  
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  for (int i = 0; i < 8; i++) {
    lcd.createChar(i, pieces[i]);
  }
}

int maze[2][16];

// run a depth-first search to validate the maze
bool vis[2][16];

void dfs(int r, int c) {
  vis[r][c] = true;
  int k = maze[r][c];
  // up
  if (r == 1 && (k == 3 || k == 4 || k == 6 || k == 7) && (maze[r - 1][c] != 2 && maze[r - 1][c] != 3 && maze[r - 1][c] != 4 && maze[r - 1][c] != 6) && !vis[r - 1][c]) {
    dfs(r - 1, c);
  }
  // down
  if (r == 0 && (k == 0 || k == 1 || k == 5 || k == 7) && (maze[r + 1][c] != 0 && maze[r + 1][c] != 1 && maze[r + 1][c] != 2 && maze[r + 1][c] != 5) && !vis[r + 1][c]) {
    dfs(r + 1, c);
  }
  // left
  if (c > 0 && (k == 1 || k == 2 || k == 4 || k == 5 || k == 6 || k == 7) && (maze[r][c - 1] != 1 && maze[r][c - 1] != 4) && !vis[r][c - 1]) {
    dfs(r, c - 1);
  }
  // right
  if (c < 15 && (k == 0 || k == 2 || k == 3 || k == 5 || k == 6 || k == 7) && (maze[r][c + 1] != 0 && maze[r][c + 1] != 3) && !vis[r][c + 1]) {
    dfs(r, c + 1);
  }
}

bool maze_ans = false;

void gen_maze() {
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 16; j++) {
      maze[i][j] = random(0, 8);
    }
  }
  memset(vis, 0, sizeof(vis));
  dfs(0, 0);
  if (vis[1][15] != maze_ans) {
    gen_maze();
  }
}

int timer;

bool time_chk() {
  return millis() - timer <= 30'000;
}

int score = 0;

int guess() {
  bool b[2] = {1, 1};
  while (b[0] && b[1]) {

    // check for time
    if (!time_chk()) {
      return -1;
    }
    
    for (int i = 0; i < 2; i++) {
      b[i] = digitalRead(14 + i);
      if (!b[i]) {
        return i;
      }
    }
    delay(50);
  }
}

void play() {
  maze_ans = random(0, 2);
  gen_maze();
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 16; j++) {
      lcd.setCursor(j, i);
      lcd.write(byte(maze[i][j]));
    }
  }
  int g = guess();
  if (g == maze_ans) {
    score++;
    tone(2, 1568, 32);
  } else if (g != -1) {
    score -= 5;
    tone(2, 740, 32);
  }
}

void loop() {
  // start screen
  lcd.clear();
  lcd.print("Press R to");
  lcd.setCursor(0, 1);
  lcd.print("begin!");

  // wait for input
  int r = 1;
  while (r) {
    r = digitalRead(15);
    delay(50);
  }
  delay(250);

  // reset score
  score = 0;

  // seed using microseconds passed
  randomSeed(micros());

  // set begin time
  timer = millis();

  while (time_chk()) {
    play();
    delay(250);
    Serial.println(score);
  }

  // result screen
  lcd.clear();
  lcd.print("Time's up!");
  lcd.setCursor(0, 1);
  lcd.print("Score: " + String(score));
  delay(5000);
}
