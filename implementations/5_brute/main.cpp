#include <iostream>
#include "sha1.hpp"
using namespace std;

const char alphabet[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
const uint64_t total = 3521614606208ULL; // 62^7 вариантов

int main() {
    string target;
    cout << "SHA1: ";
    getline(cin, target);

    for (uint64_t i = 0; i < total; ++i) {
        string password(7, '0');
        uint64_t number = i;
        for (int j = 6; j >= 0; --j) {
            password[j] = alphabet[number % 62];
            number /= 62;
        }
        if (sha1(password) == target) {
            cout << password << '\n';
            return 0;
        }
    }
    cout << "Not found\n";
}
