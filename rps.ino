/**
   Mark Zhou
   RPS
   Intensive Physical Computing + Robotics Arr. 4
   9/16/22
**/

#include <SKIF.h>

SKIF io;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  randomSeed(micros());
}

const int game[3][3] = {{ -1, 1, 0}, {0, -1, 1}, {1, 0, -1}};
const String names[3] = {"rock", "paper", "scissors"};

int get_move(String s) {
  s.toLowerCase();
  if (s == "r" || s == "rock") {
    return 0;
  }
  if (s == "p" || s == "paper") {
    return 1;
  }
  if (s == "s" || s == "scissors") {
    return 2;
  }
  return -1;
}

// https://daniel.lawrence.lu/programming/rps/
// simple decaying frequency counter

double freq[3];

int cpu() {
  // make a random choice, weighted by our frequency values
  int n = freq[0] * 1000 + freq[1] * 1000 + freq[2] * 1000;
  int r = random(0, n);
  if (r < freq[0] * 1000) {
    return 0;
  } else if (r < freq[1] * 1000) {
    return 1;
  } else {
    return 2;
  }
}

void loop() {
  // put your main code here, to run repeatedly:

  Serial.print("Welcome to RPS! How many rounds would you like the game to be? ");
  int rounds = io.readInt();
  Serial.println(rounds);

  int cpu_wins = 0, p_wins = 0;

  for (int i = 0; i < rounds; i++) {
    Serial.println("\nMATCH #" + String(i + 1) + '/' + String(rounds) + ':');
    Serial.print("Enter your move; rock, paper, or scissors (r/p/s)? ");
    String in = io.readString();
    while (get_move(in) == -1) {
      Serial.println("\nThat's not a valid move... Input rock, paper, or scissors (r/p/s). ");
      in = io.readString();
    }
    Serial.println(in);

    int cpu_mv = cpu(), p_mv = get_move(in);

    int res = game[cpu_mv][p_mv];

    Serial.print("The computer played " + names[cpu_mv] + ". ");
    if (res == 0) {
      cpu_wins++;
      Serial.println("You lost...");
    } else if (res == 1) {
      p_wins++;
      Serial.println("You won!");
    } else {
      Serial.println("It was a tie.");
    }

    // decay
    for (int i = 0; i < 3; i++) {
      freq[i] *= 0.95;
    }

    // update frequency table
    freq[(p_mv + 1) % 3] += 0.1;
    freq[(p_mv + 2) % 3] += 0.1;
  }

  Serial.println("\nFINAL SCORE:");
  Serial.println("CPU: " + String(cpu_wins));
  Serial.println("PLAYER: " + String(p_wins));

  if (cpu_wins > p_wins) {
    Serial.println("You lost the game...");
  } else if (p_wins > cpu_wins) {
    Serial.println("You won the game!");
  } else {
    Serial.println("The game was a tie.");
  }

  Serial.print('\n');
}
