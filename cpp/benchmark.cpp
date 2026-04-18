#include <iostream>
#include <chrono>
#include <vector>
#include <random>
#include <iomanip>

using namespace std;
using namespace chrono;

template <typename Func>
double timeIt(Func f) {
    auto start = high_resolution_clock::now();
    f();
    auto end = high_resolution_clock::now();
    return duration_cast<microseconds>(end - start).count() / 1000.0; // ms
}

void merge(vector<int>& arr, int left, int mid, int right, vector<int>& temp) {
    int i = left, j = mid + 1, k = left;
    while (i <= mid && j <= right) {
        if (arr[i] <= arr[j]) temp[k++] = arr[i++];
        else temp[k++] = arr[j++];
    }
    while (i <= mid) temp[k++] = arr[i++];
    while (j <= right) temp[k++] = arr[j++];
    for (int i = left; i <= right; i++) arr[i] = temp[i];
}

void sequentialMergeSort(vector<int>& arr, int left, int right, vector<int>& temp) {
    if (left >= right) return;
    int mid = left + (right - left) / 2;
    sequentialMergeSort(arr, left, mid, temp);
    sequentialMergeSort(arr, mid + 1, right, temp);
    merge(arr, left, mid, right, temp);
}

vector<int> generateRandomList(int n, int seed) {
    mt19937 gen(seed);
    uniform_int_distribution<int> dist(1, n * 10);
    vector<int> v(n);
    for (int i = 0; i < n; i++) v[i] = dist(gen);
    return v;
}

void printResult(const string& label, double timeMs) {
    cout << "  " << left << setw(40) << label;
    if (timeMs < 1.0) cout << fixed << setprecision(0) << timeMs * 1000 << "us" << endl;
    else if (timeMs < 1000.0) cout << fixed << setprecision(2) << timeMs << " ms" << endl;
    else cout << fixed << setprecision(3) << timeMs / 1000.0 << " s" << endl;
}

int main() {
    cout << endl;
    cout << "╔══════════════════════════════════════════════════════════════════╗" << endl;
    cout << "║   C++ Imperative Comparison Benchmarks                         ║" << endl;
    cout << "╚══════════════════════════════════════════════════════════════════╝" << endl;
    
    // ===== PROBLEM 1: Merge Sort =====
    cout << "\n======================================================================" << endl;
    cout << "  BENCHMARK 1: Merge Sort (std::async / std::thread)" << endl;
    cout << "======================================================================\n" << endl;
    
    for (int size : {10000, 50000, 100000, 500000}) {
        cout << "  --- Input size: " << size << " elements ---" << endl;
        auto original = generateRandomList(size, 42);
        
        { // Sequential
            auto arr = original;
            vector<int> temp(size);
            double t = timeIt([&]() {
                sequentialMergeSort(arr, 0, size - 1, temp);
            });
            printResult("Sequential Merge Sort", t);
        }
        cout << endl;
    }
}