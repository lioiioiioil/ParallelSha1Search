#include <cuda_runtime.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <algorithm>
#include "sha1.hpp"
using namespace std;

const unsigned long long total = 3521614606208ULL;
const int blocks = 9; // M = (501 + 2) % 7 + 3
const int threads_per_block = 256;
const unsigned long long batch = 1048576;

__global__ void search(const char* target, unsigned long long* answer,
                       unsigned long long begin, unsigned long long end) {
    const char alphabet[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    unsigned long long id = blockIdx.x * blockDim.x + threadIdx.x;
    unsigned long long step = gridDim.x * blockDim.x;
    for (auto i = begin + id; i < end; i += step) {
        if (atomicAdd(answer, 0ULL) != total) break;
        char password[8] = {};
        unsigned long long number = i;
        for (int j = 6; j >= 0; --j) {
            password[j] = alphabet[number % 62];
            number /= 62;
        }
        char hash[41];
        sha1(password, hash);
        bool equal = true;
        for (int j = 0; j < 40; ++j)
            if (hash[j] != target[j]) equal = false;
        if (equal) {
            atomicMin(answer, i);
            break;
        }
    }
}

void check(cudaError_t error) {
    if (error != cudaSuccess) {
        cerr << cudaGetErrorString(error) << '\n';
        exit(1);
    }
}

int main() {
    string target;
    cout << "SHA1: ";
    getline(cin, target);
    char* device_target;
    unsigned long long* device_answer;
    unsigned long long answer = total;
    check(cudaMalloc(&device_target, 41));
    check(cudaMalloc(&device_answer, sizeof(answer)));
    check(cudaMemcpy(device_target, target.c_str(), 41, cudaMemcpyHostToDevice));
    check(cudaMemcpy(device_answer, &answer, sizeof(answer), cudaMemcpyHostToDevice));
    for (unsigned long long begin = 0; begin < total && answer == total; begin += batch) {
        search<<<blocks, threads_per_block>>>(device_target, device_answer, begin, min(begin + batch, total));
        check(cudaGetLastError());
        check(cudaDeviceSynchronize());
        check(cudaMemcpy(&answer, device_answer, sizeof(answer), cudaMemcpyDeviceToHost));
    }
    check(cudaFree(device_target));
    check(cudaFree(device_answer));
    if (answer == total) cout << "Not found\n";
    else {
        const char alphabet[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
        string password(7, '0');
        for (int j = 6; j >= 0; --j) {
            password[j] = alphabet[answer % 62];
            answer /= 62;
        }
        cout << password << '\n';
    }
}
