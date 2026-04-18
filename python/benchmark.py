import time
import random
from multiprocessing import Pool

def time_it(func, *args):
    """
    Measure wall-clock time of a function call.
    Returns (result, elapsed_ms).
    """
    start = time.perf_counter()
    result = func(*args)
    elapsed = (time.perf_counter() - start) * 1000  # ms
    return result, elapsed

def print_header(title):
    print()
    print("=" * 70)
    print(f"  {title}")
    print("=" * 70)
    print()

def print_result(label, time_ms):
    if time_ms < 1.0:
        print(f"  {label:<40}{time_ms*1000:.0f} us")
    elif time_ms < 1000:
        print(f"  {label:<40}{time_ms:.2f} ms")
    else:
        print(f"  {label:<40}{time_ms/1000:.3f} s")

def merge(left, right):
    """
    Merge two sorted lists into one sorted list.
    
    COMPARISON WITH HASKELL:
      Haskell: merge [] ys = ys; merge xs [] = xs; merge (x:xs) (y:ys) = ...
      Python: Imperative with explicit indices and list building
      
    Both create a new list (neither is truly in-place).
    """
    result = []
    i = j = 0
    while i < len(left) and j < len(right):
        if left[i] <= right[j]:
            result.append(left[i])
            i += 1
        else:
            result.append(right[j])
            j += 1
    result.extend(left[i:])
    result.extend(right[j:])
    return result


def sequential_merge_sort(arr):
    """
    Sequential merge sort.
    
    COMPARISON WITH HASKELL:
      Very similar structure! Python's slice notation makes it almost as clean:
        Haskell: let (left, right) = splitAt mid xs
        Python:  left, right = arr[:mid], arr[mid:]
    """
    if len(arr) <= 1:
        return arr
    mid = len(arr) // 2
    left = sequential_merge_sort(arr[:mid])
    right = sequential_merge_sort(arr[mid:])
    return merge(left, right)

def _sort_chunk(arr):
    """Worker function for parallel sort (must be top-level for pickling)."""
    return sequential_merge_sort(arr)


def parallel_merge_sort(arr, num_workers=4):
    """
    Parallel merge sort using multiprocessing.
    
    CRITICAL DIFFERENCE FROM HASKELL:
      Python's GIL prevents true thread-level parallelism.
      We MUST use multiprocessing, which means:
      1. Each worker is a SEPARATE PROCESS (heavy to create)
      2. Data must be SERIALIZED (pickled) to send to workers
      3. Results must be DESERIALIZED when received back
      4. This IPC overhead can dominate for small inputs
      
      Haskell:
        - Sparks are nearly free (just a pointer on a work queue)
        - Data sharing via immutable memory (no copying needed)
        - GC handles cleanup automatically
    
    STRATEGY:
      1. Split array into num_workers chunks
      2. Sort each chunk in a separate process
      3. Merge sorted chunks pairwise
    """
    if len(arr) <= 1000:  # Don't parallelize small arrays
        return sequential_merge_sort(arr)
    
    chunk_size = max(1, len(arr) // num_workers)
    chunks = [arr[i:i+chunk_size] for i in range(0, len(arr), chunk_size)]
    
    with Pool(num_workers) as pool:
        sorted_chunks = pool.map(_sort_chunk, chunks)
    
    # Merge sorted chunks pairwise
    while len(sorted_chunks) > 1:
        merged = []
        for i in range(0, len(sorted_chunks), 2):
            if i + 1 < len(sorted_chunks):
                merged.append(merge(sorted_chunks[i], sorted_chunks[i+1]))
            else:
                merged.append(sorted_chunks[i])
        sorted_chunks = merged
    
    return sorted_chunks[0]

def sequential_mat_mul(a, b):
    """
    Sequential matrix multiplication using nested loops.
    
    COMPARISON WITH HASKELL:
      Haskell: [[ sum $ zipWith (*) row col | col <- transpose b ] | row <- a]
      Python:  Three nested for-loops (C-style iteration)
      
    The Haskell version is more declarative and closer to mathematical notation.
    """
    n = len(a)
    c = [[0.0] * n for _ in range(n)]
    for i in range(n):
        for k in range(n):
            for j in range(n):
                c[i][j] += a[i][k] * b[k][j]
    return c

def generate_random_list(n, seed=42):
    rng = random.Random(seed)
    return [rng.randint(1, n * 10) for _ in range(n)]

def generate_random_matrix(n, seed=42):
    rng = random.Random(seed)
    return [[rng.uniform(0, 100) for _ in range(n)] for _ in range(n)]

def main():
    print()
    print("╔══════════════════════════════════════════════════════════════════╗")
    print("║   Python Imperative Comparison Benchmarks                        ║")
    print("║   NOTE: Uses multiprocessing (not threading) due to GIL          ║")
    print("╚══════════════════════════════════════════════════════════════════╝")

    # ===== PROBLEM 1: Merge Sort =====
    print_header("BENCHMARK 1: Merge Sort (multiprocessing.Pool)")

    for size in [10000, 50000, 100000]:
        print(f"  --- Input size: {size} elements ---")
        arr = generate_random_list(size)
        
        seq_result, seq_time = time_it(sequential_merge_sort, arr)
        print_result("Sequential Merge Sort", seq_time)
        
        for workers in [2, 4]:
            par_result, par_time = time_it(parallel_merge_sort, arr, workers)
            speedup = seq_time / par_time if par_time > 0 else 0
            print_result(f"Parallel ({workers} workers)", par_time)
            print(f"    Speedup: {speedup:.2f}x")
            correct = seq_result == par_result
            print(f"    Correctness: {'PASS' if correct else 'FAIL'}")
        print()

    print_header("BENCHMARK 2: Matrix Multiplication (multiprocessing.Pool)")
    
    for size in [64, 128]:
        print(f"  --- Matrix size: {size}x{size} ---")
        a = generate_random_matrix(size, 42)
        b = generate_random_matrix(size, 99)
        
        _, seq_time = time_it(sequential_mat_mul, a, b)
        print_result("Sequential", seq_time)

if __name__ == "__main__":
    main()