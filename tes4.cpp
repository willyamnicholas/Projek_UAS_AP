#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <algorithm>

using namespace std;

// ANSI warna
#define GREEN "\033[42m"
#define YELLOW "\033[43m"
#define GRAY "\033[100m"
#define RESET "\033[0m"

// ================= EVALUASI =================
vector<int> evaluateGuess(string secret, string guess) {
    int n = secret.length();
    vector<int> result(n, 0);
    vector<bool> used(n, false);

    // hijau
    for (int i = 0; i < n; i++) {
        if (guess[i] == secret[i]) {
            result[i] = 2;
            used[i] = true;
        }
    }

    // kuning
    for (int i = 0; i < n; i++) {
        if (result[i] == 0) {
            for (int j = 0; j < n; j++) {
                if (!used[j] && guess[i] == secret[j]) {
                    result[i] = 1;
                    used[j] = true;
                    break;
                }
            }
        }
    }

    return result;
}

// ================= LOAD WORDS =================
vector<string> loadWords(string filename) {
    vector<string> words;
    ifstream file(filename);
    string word;

    if (!file) {
        cout << "File words.txt tidak ditemukan!\n";
        exit(1);
    }

    while (file >> word) {
        transform(word.begin(), word.end(), word.begin(), ::toupper);

        if (word.length() == 5) {
            words.push_back(word);
        }
    }

    return words;
}

// ================= VALIDASI =================
bool isValidWord(string guess, vector<string>& words) {
    return find(words.begin(), words.end(), guess) != words.end();
}

// ================= DRAW BOARD =================
void drawBoard(vector<string> guesses, vector<vector<int>> results, int maxTry) {
    system("cls");

    cout << "===== KATLA FINAL =====\n\n";

    for (int i = 0; i < maxTry; i++) {
        for (int j = 0; j < 5; j++) {
            if (i < guesses.size()) {
                if (results[i][j] == 2)
                    cout << GREEN << " " << guesses[i][j] << " " << RESET;
                else if (results[i][j] == 1)
                    cout << YELLOW << " " << guesses[i][j] << " " << RESET;
                else
                    cout << GRAY << " " << guesses[i][j] << " " << RESET;
            } else {
                cout << "[ ]";
            }
        }
        cout << endl;
    }
}

// ================= MAIN =================
int main() {
    srand(time(0));

    vector<string> words = loadWords("words.txt");
    string secret = words[rand() % words.size()];

    int maxTry = 6;
    vector<string> guesses;
    vector<vector<int>> results;

    while (guesses.size() < maxTry) {
        drawBoard(guesses, results, maxTry);

        string guess;
        cout << "\nMasukkan tebakan (5 huruf): ";
        cin >> guess;

        // ubah ke huruf besar
        transform(guess.begin(), guess.end(), guess.begin(), ::toupper);

        if (guess.length() != 5) {
            cout << "Harus 5 huruf!";
            cin.ignore();
            cin.get();
            continue;
        }


        vector<int> result = evaluateGuess(secret, guess);

        guesses.push_back(guess);
        results.push_back(result);

        if (guess == secret) {
            drawBoard(guesses, results, maxTry);
            cout << "\nMENANG!\n";
            return 0;
        }
    }

    drawBoard(guesses, results, maxTry);
    cout << "\nKALAH! Jawaban: " << secret << endl;

    return 0;
}