#include <iostream>
#include <chrono>
#include <vector>
#include <random>
#include <iomanip>
#include <future>
#include <map>

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

void parallelMergeSort(vector<int>& arr, int left, int right, vector<int>& temp, int depth) {
    if (left >= right) return;
    if (depth <= 0) {
        sequentialMergeSort(arr, left, right, temp);
        return;
    }
    int mid = left + (right - left) / 2;
    
    // Spawn left half on a new thread
    // NOTE: This shares 'arr' and 'temp' with the parent thread!
    // Safe ONLY because we write to non-overlapping index ranges.
    auto leftFuture = async(launch::async, [&]() {
        parallelMergeSort(arr, left, mid, temp, depth - 1);
    });
    
    // Right half on current thread
    parallelMergeSort(arr, mid + 1, right, temp, depth - 1);
    
    // Wait for left half to complete
    leftFuture.get();
    
    // Merge (sequential — both halves are now sorted)
    merge(arr, left, mid, right, temp);
}

vector<vector<double>> sequentialMatMul(const vector<vector<double>>& a,
                                        const vector<vector<double>>& b) {
    int n = a.size();
    vector<vector<double>> c(n, vector<double>(n, 0.0));
    for (int i = 0; i < n; i++)
        for (int k = 0; k < n; k++)
            for (int j = 0; j < n; j++)
                c[i][j] += a[i][k] * b[k][j];
    return c;
}

vector<vector<double>> parallelMatMul(const vector<vector<double>>& a,
                                       const vector<vector<double>>& b,
                                       int numThreads) {
    int n = a.size();
    vector<vector<double>> c(n, vector<double>(n, 0.0));
    
    vector<thread> threads;
    int chunkSize = (n + numThreads - 1) / numThreads;
    
    for (int t = 0; t < numThreads; t++) {
        int startRow = t * chunkSize;
        int endRow = min(startRow + chunkSize, n);
        
        threads.emplace_back([&, startRow, endRow]() {
            for (int i = startRow; i < endRow; i++)
                for (int k = 0; k < n; k++)
                    for (int j = 0; j < n; j++)
                        c[i][j] += a[i][k] * b[k][j];
        });
    }
    
    for (auto& t : threads) t.join();
    return c;
}

map<string, int> sequentialWordCount(const string& text) {
    map<string, int> freq;
    string word;
    for (char c : text) {
        if (isalpha(c)) {
            word += tolower(c);
        } else if (!word.empty()) {
            freq[word]++;
            word.clear();
        }
    }
    if (!word.empty()) freq[word]++;
    return freq;
}

map<string, int> parallelWordCount(const string& text, int numThreads) {
    int chunkSize = text.size() / numThreads;
    vector<future<map<string, int>>> futures;
    
    for (int t = 0; t < numThreads; t++) {
        int start = t * chunkSize;
        int end = (t == numThreads - 1) ? text.size() : (t + 1) * chunkSize;
        
        // Adjust boundaries to not split words
        while (end < (int)text.size() && isalpha(text[end])) end++;
        
        futures.push_back(async(launch::async, [&text, start, end]() {
            map<string, int> local;
            string word;
            for (int i = start; i < end; i++) {
                char c = text[i];
                if (isalpha(c)) {
                    word += tolower(c);
                } else if (!word.empty()) {
                    local[word]++;
                    word.clear();
                }
            }
            if (!word.empty()) local[word]++;
            return local;
        }));
    }
    
    // Reduce: merge all local maps
    map<string, int> result;
    for (auto& f : futures) {
        auto local = f.get();
        for (auto& [word, count] : local) {
            result[word] += count;
        }
    }
    return result;
}

vector<int> generateRandomList(int n, int seed) {
    mt19937 gen(seed);
    uniform_int_distribution<int> dist(1, n * 10);
    vector<int> v(n);
    for (int i = 0; i < n; i++) v[i] = dist(gen);
    return v;
}

vector<vector<double>> generateRandomMatrix(int n, int seed) {
    mt19937 gen(seed);
    uniform_real_distribution<double> dist(0.0, 100.0);
    vector<vector<double>> m(n, vector<double>(n));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            m[i][j] = dist(gen);
    return m;
}

string generateText(int numWords) {
    vector<string> words = {
        "the", "quick", "brown", "fox", "jumps", "over",
        "functional", "programming", "parallelism",
        "haskell", "purity", "immutability"
    };
    mt19937 gen(42);
    uniform_int_distribution<int> dist(0, words.size() - 1);
    string text;
    for (int i = 0; i < numWords; i++) {
        if (i > 0) text += " ";
        text += words[dist(gen)];
    }
    return text;
}

void printResult(const string& label, double timeMs) {
    cout << "  " << left << setw(40) << label;
    if (timeMs < 1.0) cout << fixed << setprecision(0) << timeMs * 1000 << "us" << endl;
    else if (timeMs < 1000.0) cout << fixed << setprecision(2) << timeMs << " ms" << endl;
    else cout << fixed << setprecision(3) << timeMs / 1000.0 << " s" << endl;
}

int main() {
    cout << endl;
    cout << "C++ Imperative Comparison Benchmarks" << endl;
    
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
            // Parallel
            for (int depth : {2, 3, 4}) {
                auto arr2 = original;
                vector<int> temp2(size);
                double pt = timeIt([&]() {
                    parallelMergeSort(arr2, 0, size - 1, temp2, depth);
                });
                string label = "Parallel (depth=" + to_string(depth) + ")";
                printResult(label, pt);
                cout << "    Speedup: " << fixed << setprecision(2) << t / pt << "x" << endl;
                
                // Verify correctness
                bool correct = (arr == arr2);
                cout << "    Correctness: " << (correct ? "PASS" : "FAIL") << endl;
            }
        }
        cout << endl;
    }
    // ===== PROBLEM 2: Matrix Multiplication =====
    cout << "======================================================================" << endl;
    cout << "  BENCHMARK 2: Matrix Multiplication (std::thread)" << endl;
    cout << "======================================================================\n" << endl;
    
    for (int size : {64, 128, 256}) {
        cout << "  --- Matrix size: " << size << "x" << size << " ---" << endl;
        auto a = generateRandomMatrix(size, 42);
        auto b = generateRandomMatrix(size, 99);
        
        double seqTime;
        {
            seqTime = timeIt([&]() { sequentialMatMul(a, b); });
            printResult("Sequential", seqTime);
        }

        for (int threads : {2, 4, 8}) {
            double pt = timeIt([&]() { parallelMatMul(a, b, threads); });
            string label = "Parallel (" + to_string(threads) + " threads)";
            printResult(label, pt);
            cout << "    Speedup: " << fixed << setprecision(2) << seqTime / pt << "x" << endl;
        }
        cout << endl;
    }

    // ===== PROBLEM 3: Word Count =====
    cout << "======================================================================" << endl;
    cout << "  BENCHMARK 3: Word Count (std::async)" << endl;
    cout << "======================================================================\n" << endl;
    
    for (int numWords : {155000, 465000}) {
        cout << "  --- Text size: ~" << numWords << " words ---" << endl;
        string text = generateText(numWords);
        
        double seqTime = timeIt([&]() { sequentialWordCount(text); });
        printResult("Sequential", seqTime);

        for (int threads : {2, 4, 8}) {
            double pt = timeIt([&]() { parallelWordCount(text, threads); });
            string label = "Parallel (" + to_string(threads) + " threads)";
            printResult(label, pt);
            cout << "    Speedup: " << fixed << setprecision(2) << seqTime / pt << "x" << endl;
        }
        cout << endl;
    }



    cout << "\nAll C++ benchmarks complete!" << endl;
    return 0;
}