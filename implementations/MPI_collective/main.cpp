#include <mpi.h>
#include <iostream>
#include <algorithm>
#include "sha1.hpp"
using namespace std;

const char alphabet[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
const unsigned long long total = 3521614606208ULL;
const int processes = 9;
const unsigned long long batch = 4096 * processes;

string password_at(unsigned long long number) {
    string password(7, '0');
    for (int j = 6; j >= 0; --j) {
        password[j] = alphabet[number % 62];
        number /= 62;
    }
    return password;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank, count;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &count);
    if (count != processes) {
        if (rank == 0) cerr << "Run with 9 MPI processes\n";
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    char hash[41] = {};
    if (rank == 0) {
        string target;
        cout << "SHA1: ";
        getline(cin, target);
        target.copy(hash, 40);
    }
    MPI_Bcast(hash, 41, MPI_CHAR, 0, MPI_COMM_WORLD);
    unsigned long long answer = total; // total означает «пока не найдено».
    for (unsigned long long begin = 0; begin < total; begin += batch) {
        unsigned long long local = total;
        for (auto i = begin + rank; i < min(begin + batch, total); i += processes) {
            if (sha1(password_at(i)) == hash) { local = i; break; }
        }
        MPI_Allreduce(&local, &answer, 1, MPI_UNSIGNED_LONG_LONG, MPI_MIN, MPI_COMM_WORLD);
        if (answer != total) break;
    }
    if (rank == 0) cout << (answer == total ? "Not found" : password_at(answer)) << '\n';
    MPI_Finalize();
}
