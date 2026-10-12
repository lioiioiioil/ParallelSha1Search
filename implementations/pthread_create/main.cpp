#include <iostream>
#include <atomic>
#include <pthread.h>
#include <cstdlib>
#include "sha1.hpp"
using namespace std;

const char alphabet[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
const uint64_t total = 3521614606208ULL; // 62^7 вариантов
const int threads = 9;
string target, result;
atomic<bool> found(false);

void search(int id) {
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

void* worker(void* parameter) {
    search(*static_cast<int*>(parameter));
    return nullptr;
}

int main() {
    cout << "SHA1: ";
    getline(cin, target);
    pthread_t workers[threads];
    int ids[threads];
    for (int i = 0; i < threads; ++i) {
        ids[i] = i;
        int error = pthread_create(&workers[i], nullptr, worker, &ids[i]);
        if (error != 0) {
            cerr << "pthread_create error: " << error << '\n';
            exit(1);
        }
    }
    for (int i = 0; i < threads; ++i) {
        int error = pthread_join(workers[i], nullptr);
        if (error != 0) {
            cerr << "pthread_join error: " << error << '\n';
            exit(1);
        }
    }
    cout << (found ? result : "Not found") << '\n';
}
