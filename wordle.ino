#include "wordbank.h"
#include <time.h>

String ans, guess;
bool in_ans[26];

bool valid_guess(String s) {
  if (s.length() != 5) {
    return false;
  }
  for (int i = 0; i < BANK_SZ; i++) {
    if (s == bank[i]) {
      return true;
    }
  }
  return false;
};

void ask_guess() {
  while (Serial.available() <= 0) {}
  guess = Serial.readString();
  if (!valid_guess(guess)) {
    Serial.println("That word isn't 5 letters long or isn't in the bank... try again.");
    ask_guess();
    return;
  }
  Serial.println(guess);
  for (int i = 0; i < 5; i++) {
    if (guess[i] == ans[i]) {
      Serial.print('G');
    } else if (in_ans[guess[i] - 'a']) {
      Serial.print('Y');
    } else {
      Serial.print('X');
    }
  }
  Serial.print("\n\n");
}

void setup() {
  Serial.begin(9600);

  unsigned long u_sec = micros();
  randomSeed(u_sec);
}

void loop() {

  ans = ask[random(0, ASK_SZ)];
  memset(in_ans, 0, sizeof(in_ans));
  for (int i = 0; i < 5; i++) {
    in_ans[ans[i] - 'a'] = true;
  }
  
  Serial.println("The word has been picked! 5 guesses, 5 letter words only, G is green, Y is yellow, and X is gray. Good luck!\n");

  bool win = false;
  for (int i = 0; i < 5; i++) {
    Serial.println("Guess #" + String(i + 1) + ':');
    ask_guess();
    if (guess == ans) {
      win = true;
      break;
    }
  }

  if (win) {
    Serial.println("Congratulations! You found the word, \"" + ans + "\".");
  } else {
    Serial.println("Better luck next time! The word was \"" + ans + "\".");
  }
  Serial.print('\n');
}
