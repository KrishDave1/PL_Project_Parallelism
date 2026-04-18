import time
import random



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

def generate_random_list(n, seed=42):
    rng = random.Random(seed)
    return [rng.randint(1, n * 10) for _ in range(n)]


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
        