#include <iostream>
#include <atomic>
#include <thread>
#include "sha1.hpp"
using namespace std;

const char alphabet[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
const uint64_t total = 3521614606208ULL; // 62^7 вариантов
const int threads = 9;
string target, result;
atomic<bool> found(false);

void search_password(int id) {
    for (uint64_t i = id; i < total && !found; i += threads) {
        string password(7, '0');
        uint64_t number = i;
        for (int j = 6; j >= 0; --j) {
            password[j] = alphabet[number % 62];
            number /= 62;
        }
        if (sha1(password) == target) {
            if (!found.exchange(true)) result = password;
            break;
        }
    }
}

int main() {
    cout << "SHA1: ";
    getline(cin, target);
    thread workers[threads];
    try {
        for (int i = 0; i < threads; ++i) workers[i] = thread(search_password, i);
    } catch (const system_error& error) {
        found = true;
        for (auto& worker : workers)
            if (worker.joinable()) worker.join();
        cerr << error.what() << '\n';
        return 1;
    }
    for (auto& worker : workers) worker.join();
    cout << (found ? result : "Not found") << '\n';
}
