#include <iostream>
#include <atomic>
#include <omp.h>
#include "sha1.hpp"
using namespace std;

#ifndef _OPENMP
#error Compile with OpenMP enabled
#endif

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

int main() {
    cout << "SHA1: ";
    getline(cin, target);

    int count = 0;
    omp_set_dynamic(0);
    #pragma omp parallel num_threads(threads)
    {
        #pragma omp single
        count = omp_get_num_threads();
        if (count == threads) search(omp_get_thread_num());
    }
    if (count != threads) {
        cout << "Cannot start 9 threads\n";
        return 1;
    }
    cout << (found ? result : "Not found") << '\n';
}
