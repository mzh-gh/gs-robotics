// Pinecone
// A rock-paper-scissors engine
// Mark Zhou
// v1.1, 9/25/22

// Pastebin:
// https://pastebin.com/H6ncJNTT

// [v1.0]: Initial release
//         Play: https://www.online-cpp.com/7iTZ3ydjvO (9/23/22)
// [v1.1]: There is no longer an artificial limit on the number of rounds. The engine will forget the oldest move
//         when it runs out of memory (super efficiently!) + some extra optimizations for predictor 3.
//         Play: https://www.online-cpp.com/uvj7n8zwSt (9/25/22)

// Don't set your expectations too high, since rock paper scissors is still mostly a game of luck,
// and it's not possible to consistently beat a random player. Fortunately, humans usually
// aren't very good at being random, and so I think this program could beat most people about 80%
// of the time when playing games with 25+ matches, which isn't bad.

#include <chrono>
#include <cmath>
#include <cstring>
#include <iostream>
#include <random>

using namespace std;

mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
#define random(a, b) uniform_int_distribution<int>(a, b - 1)(rng)

// Predictors:
// 0: general frequency counter (they may have a tendency to play some moves more than others)
// 1: frequency counter vs. our last move (they may have a tendency to play move y in response to move x)
// 2: frequency counter vs. their last move (they may have a tendency to play move y after move x)
// 3: substring matching vs their moves (effective vs. long patterns)
// 4: random as a fallback (random can't lose consistently, may be a good defense vs. a targeted attack)

// Predictor selection:
// Make each predictor choose a value before the match, and increment its weight if it won,
// decrement if it lost. Pick the predictor with the maximum weight.

// Additional heuristic idea: move time (NOT IMPLEMENTED)
// If the player spent less than 0.5s making the decision, decrease the weight of the frequency
// counter vs. our last move, as there wouldn't be enough time to make a decision like that.

// Debug mode makes the engine print out the predictors' weight values
constexpr bool DEBUG_MODE = false;

// The max number of rounds the engine can remember and search through (for predictor 3).
// The string search is O(HISTORY_SZ^3), so keeping this low may help the program stay fast at high round numbers.
constexpr int HISTORY_SZ = 250;

// Predictor selection
long double predictor_weight[5];

// General frequency
long double general_freq[3];

int opp, opp_prev;
int cpu, cpu_prev;

// [last move][probability of each]
long double reactive_freq[3][3], pattern_freq[3][3];

int round_num;
int history[HISTORY_SZ];

int predictor_moves[5];

void init() {
    memset(predictor_weight, 0, sizeof predictor_weight);
    memset(general_freq, 0, sizeof general_freq);
    opp_prev = -1;
    cpu_prev = -1;
    memset(reactive_freq, 0, sizeof reactive_freq);
    memset(pattern_freq, 0, sizeof pattern_freq);
    round_num = 0;
    memset(history, -1, sizeof history);
}

// Generate moves from the 5 predictors
void gen_moves() {
    // General frequency counter, weighted by e^(frequency value)
    int p0_r = exp(general_freq[0]) * 1000;
    int p0_p = exp(general_freq[1]) * 1000;
    int p0_s = exp(general_freq[2]) * 1000;
    int p0_rand = random(0, int(p0_r + p0_p + p0_s));
    if (p0_rand < p0_r) {
        predictor_moves[0] = 0;
    } else if (p0_rand < p0_r + p0_s) {
        predictor_moves[0] = 1;
    } else {
        predictor_moves[0] = 2;
    }
    // Frequency counter vs. our last move, weighted by e^(frequency value)
    if (cpu_prev != -1) {
        int p1_r = exp(reactive_freq[cpu_prev][0]) * 1000;
        int p1_p = exp(reactive_freq[cpu_prev][1]) * 1000;
        int p1_s = exp(reactive_freq[cpu_prev][2]) * 1000;
        int p1_rand = random(0, int(p1_r + p1_p + p1_s));
        if (p1_rand < p1_r) {
            predictor_moves[1] = 0;
        } else if (p1_rand < p1_r + p1_s) {
            predictor_moves[1] = 1;
        } else {
            predictor_moves[1] = 2;
        }
    } else {
        predictor_moves[1] = random(0, 3);
    }
    // Frequency counter vs. their last move, weighted by e^(frequency value)
    if (opp_prev != -1) {
        int p2_r = exp(pattern_freq[opp_prev][0]) * 1000;
        int p2_p = exp(pattern_freq[opp_prev][1]) * 1000;
        int p2_s = exp(pattern_freq[opp_prev][2]) * 1000;
        int p2_rand = random(0, int(p2_r + p2_p + p2_s));
        if (p2_rand < p2_r) {
            predictor_moves[2] = 0;
        } else if (p2_rand < p2_r + p2_s) {
            predictor_moves[2] = 1;
        } else {
            predictor_moves[2] = 2;
        }
    } else {
        predictor_moves[2] = random(0, 3);
    }
    // Substring matching
    for (int i = min(round_num, HISTORY_SZ) - 1; i >= 1; i--) {
        for (int j = round_num - i - 1; j >= round_num - 1 - HISTORY_SZ + 1; j--) {
            bool match = true;
            for (int k = 0; k < i; k++) {
                if (!(history[(j + k + HISTORY_SZ) % HISTORY_SZ] == history[(round_num - i + k + HISTORY_SZ) % HISTORY_SZ])) {
                    match = false;
                    break;
                }
            }
            if (match) {
                predictor_moves[3] = (history[j + i] + 1) % 3;
                goto exit_loop;
            }
        }
    }
    exit_loop:;
    // Random choice
    predictor_moves[4] = random(0, 3);
}

// Choose the move with the highest weight
void choose() {
    int best = 0;
    for (int i = 1; i < 5; i++) {
        if (predictor_weight[i] > predictor_weight[best]) {
            best = i;
        }
    }
    cpu = predictor_moves[best];
    if (DEBUG_MODE) {
        cout << '\n';
        cout << "SELECTED PREDICTOR: " << best << '\n';
        for (int i = 0; i < 5; i++) {
            cout << predictor_weight[i] << ' ';
        }
        cout << '\n';
    }
    cout << '\n';
}

// Evaluate the moves of each predictor & decay score
void eval_moves() {
    // Decay predictor selector
    for (int i = 0; i < 5; i++) {
        predictor_weight[i] *= 0.85;
    }
    // Decay all frequency predictors
    for (int i = 0; i < 3; i++) {
        general_freq[i] *= 0.85;
        for (int j = 0; j < 3; j++) {
            reactive_freq[i][j] *= 0.85;
            pattern_freq[i][j] *= 0.85;
        }
    }
    // Update predictor selector weight
    for (int i = 0; i < 5; i++) {
        if ((predictor_moves[i] + 2) % 3 == opp) {
            predictor_weight[i] += 0.1;
        } else if ((predictor_moves[i] + 1) % 3 == opp) {
            predictor_weight[i] -= 0.1;
        }
    }
    // Update general frequency counter
    general_freq[(opp + 1) % 3] += 0.1;
    general_freq[(opp + 2) % 3] -= 0.1;
    // Update reactive frequency counter
    if (cpu_prev != -1) {
        reactive_freq[cpu_prev][(opp + 1) % 3] += 0.1;
        reactive_freq[cpu_prev][(opp + 2) % 3] -= 0.1;
    }
    // Update pattern frequency counter
    if (opp_prev != -1) {
        pattern_freq[opp_prev][(opp + 1) % 3] += 0.1;
        pattern_freq[opp_prev][(opp + 2) % 3] -= 0.1;
    }
    // Update history
    history[round_num++] = opp;
    // Update previous moves
    opp_prev = opp;
    cpu_prev = cpu;
}

int main() {
    cin.exceptions(cin.failbit);

    cout << "\nPinecone v1.0\n";
    cout << "Mark Zhou 9/23/22\n\n";

    cout << "How many rounds would you like the game to be? ";
    int rounds;
    cin >> rounds;
    while (rounds < 1) {
        cout << "That's not a valid round number... Please try again: ";
        cin >> rounds;
    }
    int cpu_wins = 0, opp_wins = 0;
    for (int i = 0; i < rounds; i++) {
        cout << "\nROUND " << i + 1 << " OF " << rounds << ":\n";
        gen_moves();
        choose();
        cout << "Enter a move: 0 for rock, 1 for paper, and 2 for scissors: ";
        cin >> opp;
        while (opp < 0 || opp > 2) {
            cout << "That's not a valid move... Please try again: ";
            cin >> opp;
        }
        cout << "The CPU played " << cpu << ". ";
        if ((cpu + 2) % 3 == opp) {
            cout << "You lost...\n";
            cpu_wins++;
        } else if ((cpu + 1) % 3 == opp) {
            cout << "You won!\n";
            opp_wins++;
        } else {
            cout << "It was a tie.\n";
        }
        eval_moves();
        cout << "CPU: " << cpu_wins << '\n';
        cout << "You: " << opp_wins << '\n';
    }
    cout << "\nFINAL RESULT:\n";
    if (cpu_wins > opp_wins) {
        cout << "The CPU won " << cpu_wins << " to " << opp_wins << ".\n";
    } else if (opp_wins > cpu_wins) {
        cout << "You won " << opp_wins << " to " << cpu_wins << ".\n";
    } else {
        cout << "You tied with the CPU " << cpu_wins << " to " << opp_wins << ".\n";
    }
}
