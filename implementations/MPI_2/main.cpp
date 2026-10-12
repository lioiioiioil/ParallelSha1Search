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
    if (rank == 0) {
        for (int id = 1; id < processes; ++id)
            MPI_Send(hash, 41, MPI_CHAR, id, 0, MPI_COMM_WORLD);
    } else MPI_Recv(hash, 41, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    unsigned long long answer = total; // total означает «пока не найдено».
    for (unsigned long long begin = 0; begin < total; begin += batch) {
        unsigned long long local = total;
        for (auto i = begin + rank; i < min(begin + batch, total); i += processes) {
            if (sha1(password_at(i)) == hash) { local = i; break; }
        }
        if (rank == 0) {
            answer = local;
            for (int id = 1; id < processes; ++id) {
                unsigned long long received;
                MPI_Recv(&received, 1, MPI_UNSIGNED_LONG_LONG, id, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                answer = min(answer, received);
            }
            for (int id = 1; id < processes; ++id)
                MPI_Send(&answer, 1, MPI_UNSIGNED_LONG_LONG, id, 2, MPI_COMM_WORLD);
        } else {
            MPI_Send(&local, 1, MPI_UNSIGNED_LONG_LONG, 0, 1, MPI_COMM_WORLD);
            MPI_Recv(&answer, 1, MPI_UNSIGNED_LONG_LONG, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        if (answer != total) break;
    }
    if (rank == 0) cout << (answer == total ? "Not found" : password_at(answer)) << '\n';
    MPI_Finalize();
}
