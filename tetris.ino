#include "constants.h"

//#include <Adafruit_GFX.h>    // Core graphics library
#include <ST7735_t3.h> // Hardware-specific library
#include <ST7789_t3.h> // Hardware-specific library

// EEPROM for writing data persistently (for high score)
#include <EEPROM.h>

ST7735_t3 tft = ST7735_t3(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCK, TFT_RST);

// 0 = empty
// 1 = dormant tile
// 2 = moving tile
int grid[10][20];
int temp[10][20];

int cur_x = 2, cur_y = 0, score = 0;
int cur_p = 2, cur_r = 1;

// Tetris scoring table:
// 1: 40
// 2: 100
// 3: 300
// 4: 1200

void clear_lines() {
  memset(temp, 0, sizeof temp);
  int clears = 0;
  int pt = 19;
  for (int i = 19; i >= 0; i--) {
    bool full = true;
    for (int j = 0; j < 10; j++) {
      full &= grid[j][i] == 1;
    }
    if (full) {
      clears++;
    } else {
      for (int j = 0; j < 10; j++) {
        temp[j][pt] = grid[j][i];
      }
      pt--;
    }
  }
  if (clears == 1) {
    score += 40;
  } else if (clears == 2) {
    score += 100;
  } else if (clears == 3) {
    score += 300;
  } else if (clears == 4) {
    score += 1200;
  }
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 20; j++) {
      grid[i][j] = temp[i][j];
    }
  }
}

void setup(void) {
  Serial.begin(9600);
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST7735_BLACK);
  //  grid[0][0] = 2;

  pinMode(A9, INPUT);
  pinMode(A8, INPUT);
  pinMode(A7, INPUT_PULLUP);

  setup_board();
}

void draw_board() {
  tft.drawRect(0, 0, 128, 160, ST7735_WHITE);
  tft.drawRect(8, 8, 72, 142, ST7735_RED);

  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 20; j++) {
      if (grid[i][j] != 0) {
        tft.drawRect(9 + i * 7, 9 + j * 7, 7, 7, ST7735_BLUE);
        tft.fillRect(10 + i * 7, 10 + j * 7, 5, 5, ST7735_WHITE);
      } else {
        tft.fillRect(9 + i * 7, 9 + j * 7, 7, 7, ST7735_BLACK);
      }
    }
  }
}

void write_piece(int v) {
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      if (pieces[cur_p][cur_r][i][j]) {
        grid[cur_x + i][cur_y + j] = v;
      }
    }
  }
}

void upd_score(uint16_t color) {
  tft.setTextColor(color);
  tft.setCursor(82, 16);
  tft.print(score);
}

bool check_pos(int x, int y, int p, int r) {
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      if (pieces[p][r][i][j]) {
        if (x + i < 0 || x + i >= 10 || y + j < 0 || y + j >= 20 || grid[x + i][y + j] == 1) {
          return false;
        }
      }
    }
  }
  return true;
}

int state = 0;

void write_centered(int y, String s) {
  int n = s.length();
  int s_pixels = n * 5 + n - 1;
  int l_border = (128 - s_pixels) / 2;
  tft.setTextColor(ST7735_WHITE);
  tft.setCursor(l_border, y);
  tft.print(s);
}

void setup_board() {
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 20; j++) {
      grid[i][j] = 0;
      temp[i][j] = 0;
    }
  }
  cur_x = 4;
  cur_y = 0;
  score = 0;
  randomSeed(micros());
  cur_p = random(0, 7);
  cur_r = 0;
  state = 0;
  
  tft.fillScreen(ST7735_BLACK);
  tft.setTextColor(ST7735_WHITE);
  tft.setCursor(82, 8);
  tft.print("Score:");
}

// Read 32-bit integer from the EEPROM
// Needed because an EEPROM address only stores 1 byte
// Use addresses [0, 4) like this
// 00000000 00000000 00000000 00000000
int read_topscore() {
  int res = 0;
  for (int i = 0; i < 4; i++) {
    res += EEPROM.read(i) << (i * 8);
  }
  return res;
}

void write_topscore() {
  for (int i = 0; i < 4; i++) {
    EEPROM.write(i, score >> (i * 8));
  }
}

void loop() {
  if (!check_pos(cur_x, cur_y, cur_p, cur_r)) {
    
    // Lose screen
    tft.fillScreen(ST7735_BLACK);
    tft.setTextColor(ST7735_WHITE);
    write_centered(10, "Game Over!");
    write_centered(25, "Your score was:");
    write_centered(35, String(score));
    write_centered(50, "Top score:");
    write_centered(60, String(read_topscore()));
    write_centered(75, "Press the joystick");
    write_centered(85, "down to try again.");
    while (digitalRead(A7) == 1) {}

    // Write to EEPROM
    // EEPROM.write(0, score);
    if (score > read_topscore()) {
      write_topscore();
    }

    // Reset everything
    setup_board();
  }

  write_piece(2);
  draw_board();

  // String mv = io.readString();
  // mv.toLowerCase();

  int read_x = analogRead(A9);
  int read_y = analogRead(A8);
  int read_but = digitalRead(A7);

  write_piece(0);

  if (grid[cur_x][cur_y] == 2)
    grid[cur_x][cur_y] = 0;

  // if (mv == "n" && cur_y > 0 && check_pos(cur_x, cur_y - 1, cur_p, cur_r)) {
  //   cur_y--;
  // }
  if (state % 2 == 0 && read_x > 1000 && check_pos(cur_x + 1, cur_y, cur_p, cur_r)) {
    cur_x++;
  }
  if ((read_y > 1000 || state == 19) && check_pos(cur_x, cur_y + 1, cur_p, cur_r)) {
    cur_y++;
  }
  if (state % 2 == 0 && read_x < 15 && check_pos(cur_x - 1, cur_y + 1, cur_p, cur_r)) {
    cur_x--;
  }
  if (state % 2 == 0 && read_but == 0 && check_pos(cur_x, cur_y, cur_p, (cur_r + 1) % 4)) {
    cur_r = (cur_r + 1) % 4;
  }

  upd_score(ST7735_BLACK);
  if (!check_pos(cur_x, cur_y + 1, cur_p, cur_r)) {
    write_piece(1);
    clear_lines();
    cur_x = 4;
    cur_y = 0;
    cur_p = random(0, 7);
    cur_r = 0;
  }
  upd_score(ST7735_WHITE);

  state = (state + 1) % 20;
  delay(50);
}