#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <atomic>
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

DWORD WINAPI worker(LPVOID parameter) {
    int id = *static_cast<int*>(parameter);
    search(id);
    return 0;
}

int main() {
    cout << "SHA1: ";
    getline(cin, target);

    HANDLE handles[threads];
    int ids[threads];
    for (int i = 0; i < threads; ++i) {
        ids[i] = i;
        handles[i] = CreateThread(nullptr, 0, worker, &ids[i], 0, nullptr);
        if (!handles[i]) {
            cerr << "CreateThread error: " << GetLastError() << '\n';
            ExitProcess(1);
        }
    }
    if (WaitForMultipleObjects(threads, handles, TRUE, INFINITE) != WAIT_OBJECT_0) {
        cerr << "Wait error: " << GetLastError() << '\n';
        ExitProcess(1);
    }
    for (int i = 0; i < threads; ++i) CloseHandle(handles[i]);
    cout << (found ? result : "Not found") << '\n';
}
