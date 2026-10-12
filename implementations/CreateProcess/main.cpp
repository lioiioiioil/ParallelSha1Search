#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <iostream>
#include "sha1.hpp"
using namespace std;

const char alphabet[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
const uint64_t total = 3521614606208ULL;
const int processes = 9;

struct Shared {
    volatile LONG found;
    char result[8];
};

void search(int id, const string& target, Shared* shared) {
    for (uint64_t i = id; i < total; i += processes) {
        if (InterlockedCompareExchange(&shared->found, 0, 0)) break;
        string password(7, '0');
        uint64_t number = i;
        for (int j = 6; j >= 0; --j) {
            password[j] = alphabet[number % 62];
            number /= 62;
        }
        if (sha1(password) == target) {
            if (InterlockedCompareExchange(&shared->found, 1, 0) == 0)
                password.copy(shared->result, 7);
            break;
        }
    }
}

int main(int argc, char** argv) {
    string target;
    HANDLE mapping;
    if (argc == 4) { // Дочерний запуск: номер, хеш, унаследованный дескриптор.
        int id = stoi(argv[1]);
        target = argv[2];
        mapping = reinterpret_cast<HANDLE>(static_cast<uintptr_t>(stoull(argv[3])));
        auto* shared = static_cast<Shared*>(MapViewOfFile(mapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Shared)));
        if (!shared) return 1;
        search(id, target, shared);
        UnmapViewOfFile(shared);
        CloseHandle(mapping);
        return 0;
    }
    cout << "SHA1: ";
    getline(cin, target);
    SECURITY_ATTRIBUTES security = {sizeof(security), nullptr, TRUE};
    mapping = CreateFileMappingA(INVALID_HANDLE_VALUE, &security, PAGE_READWRITE, 0, sizeof(Shared), nullptr);
    if (!mapping) { cerr << "CreateFileMapping error\n"; return 1; }
    auto* shared = static_cast<Shared*>(MapViewOfFile(mapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Shared)));
    if (!shared) { CloseHandle(mapping); return 1; }
    // Новая область памяти уже заполнена нулями: found = 0, result пустой.
    char executable[32768];
    DWORD length = GetModuleFileNameA(nullptr, executable, sizeof(executable));
    if (length == 0 || length >= sizeof(executable)) {
        UnmapViewOfFile(shared); CloseHandle(mapping); return 1;
    }
    HANDLE children[processes - 1];
    int created = 0;
    bool error = false;
    for (int id = 1; id < processes; ++id) {
        string command = "\"" + string(executable) + "\" " + to_string(id) + " " + target + " "
                       + to_string(reinterpret_cast<uintptr_t>(mapping));
        STARTUPINFOA startup{};
        startup.cb = sizeof(startup);
        PROCESS_INFORMATION process{};
        if (!CreateProcessA(executable, command.data(), nullptr, nullptr, TRUE, 0, nullptr, nullptr, &startup, &process)) {
            cerr << "CreateProcess error: " << GetLastError() << '\n';
            InterlockedExchange(&shared->found, 1);
            error = true;
            break;
        }
        children[created++] = process.hProcess;
        CloseHandle(process.hThread);
    }
    if (!error) search(0, target, shared); // Родитель + 8 дочерних = 9 процессов.
    for (int i = 0; i < created; ++i) {
        if (WaitForSingleObject(children[i], INFINITE) != WAIT_OBJECT_0) {
            InterlockedExchange(&shared->found, 1);
            cerr << "Wait error\n";
            ExitProcess(1);
        }
        DWORD code = 1;
        if (!GetExitCodeProcess(children[i], &code) || code != 0) error = true;
        CloseHandle(children[i]);
    }
    if (!error) cout << (shared->found ? shared->result : "Not found") << '\n';
    else cerr << "Child process error\n";
    UnmapViewOfFile(shared);
    CloseHandle(mapping);
    return error ? 1 : 0;
}
