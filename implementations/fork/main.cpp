#include <iostream>
#include <atomic>
#include <new>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include "sha1.hpp"
using namespace std;

const char alphabet[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
const uint64_t total = 3521614606208ULL;
const int processes = 9;
string target;

struct Shared {
    atomic<int> found{0};
    char result[8]{};
};
// Для общей памяти между процессами нужны атомики без внутренних блокировок.
static_assert(atomic<int>::is_always_lock_free);

void search(int id, Shared* shared) {
    for (uint64_t i = id; i < total && !shared->found; i += processes) {
        string password(7, '0');
        uint64_t number = i;
        for (int j = 6; j >= 0; --j) {
            password[j] = alphabet[number % 62];
            number /= 62;
        }
        if (sha1(password) == target) {
            if (!shared->found.exchange(1))
                password.copy(shared->result, 7);
            break;
        }
    }
}

int main() {
    cout << "SHA1: ";
    getline(cin, target);
    void* memory = mmap(nullptr, sizeof(Shared), PROT_READ | PROT_WRITE,
                        MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (memory == MAP_FAILED) { perror("mmap"); return 1; }
    Shared* shared = new (memory) Shared;
    pid_t children[processes - 1];
    int created = 0;
    bool error = false;
    for (int id = 1; id < processes; ++id) {
        pid_t pid = fork();
        if (pid == 0) {
            search(id, shared);
            _exit(0);
        }
        if (pid < 0) {
            perror("fork");
            shared->found = 1;
            error = true;
            break;
        }
        children[created++] = pid;
    }
    if (!error) search(0, shared); // Родитель тоже ищет: всего 9 процессов.
    for (int i = 0; i < created; ++i) {
        int status = 0;
        pid_t waited;
        do { waited = waitpid(children[i], &status, 0); } while (waited < 0 && errno == EINTR);
        if (waited < 0 || !WIFEXITED(status) || WEXITSTATUS(status) != 0) error = true;
    }
    if (!error) cout << (shared->found ? shared->result : "Not found") << '\n';
    else cerr << "Child process error\n";
    shared->~Shared();
    munmap(memory, sizeof(Shared));
    return error ? 1 : 0;
}
