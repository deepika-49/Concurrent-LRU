import time
import threading


class Node:

  def __init__(self, key=0, value=0):
    self.key = key
    self.value = value
    self.prev = None
    self.next = None


class LRUCache:

  def __init__(self, capacity: int):
    self.capacity = max(0, capacity)
    self.cache = {}
    self.lock = threading.Lock()

    self.head = Node()
    self.tail = Node()
    self.head.next = self.tail
    self.tail.prev = self.head

  def _add_node(self, node: Node):
    node.prev = self.head
    node.next = self.head.next
    self.head.next.prev = node
    self.head.next = node

  def _remove_node(self, node: Node):
    prev_node = node.prev
    next_node = node.next
    prev_node.next = next_node
    next_node.prev = prev_node

  def _move_to_head(self, node: Node):
    self._remove_node(node)
    self._add_node(node)

  def _pop_tail(self) -> Node:
    res = self.tail.prev
    self._remove_node(res)
    return res

  def get(self, key: int) -> int:
    with self.lock:
      node = self.cache.get(key)
      if not node:
        return -1
      self._move_to_head(node)
      return node.value

  def put(self, key: int, value: int) -> None:
    if self.capacity <= 0:
      return

    with self.lock:
      node = self.cache.get(key)
      if not node:
        new_node = Node(key, value)
        self.cache[key] = new_node
        self._add_node(new_node)
        if len(self.cache) > self.capacity:
          tail = self._pop_tail()
          del self.cache[tail.key]
      else:
        node.value = value
        self._move_to_head(node)


class NaiveLRUCache:

  def __init__(self, capacity: int):
    self.capacity = capacity
    self.cache = {}
    self.lock = threading.Lock()

  def get(self, key: int) -> int:
    with self.lock:
      if key not in self.cache:
        return -1
      val = self.cache.pop(key)
      self.cache[key] = val
      return val

  def put(self, key: int, value: int) -> None:
    if self.capacity <= 0:
      return
    with self.lock:
      if key in self.cache:
        self.cache.pop(key)
      elif len(self.cache) >= self.capacity:
        first_key = next(iter(self.cache))
        del self.cache[first_key]
      self.cache[key] = value


def run_unit_tests():
  cache_zero = LRUCache(0)
  cache_zero.put(1, 1)
  assert cache_zero.get(1) == -1, "Edge Case Failed: Capacity 0"

  cache_one = LRUCache(1)
  cache_one.put(1, 10)
  assert cache_one.get(1) == 10, "Edge Case Failed: Capacity 1 insert"
  cache_one.put(2, 20)
  assert cache_one.get(1) == -1, "Edge Case Failed: Capacity 1 eviction"
  assert cache_one.get(2) == 20, "Edge Case Failed: Capacity 1 lookup"

  cache = LRUCache(2)
  cache.put(1, 1)
  cache.put(2, 2)
  assert cache.get(1) == 1, "Test 1 Failed"
  cache.put(3, 3)
  assert cache.get(2) == -1, "Test 2 Failed (LRU Eviction)"
  cache.put(4, 4)
  assert cache.get(1) == -1, "Test 3 Failed (LRU Eviction)"
  assert cache.get(3) == 3, "Test 4 Failed"
  assert cache.get(4) == 4, "Test 5 Failed"
  cache.put(4, 40)
  assert cache.get(4) == 40, "Test 6 Failed (Update existing key)"


def run_stress_test():
  capacity = 50
  cache = LRUCache(capacity)
  num_threads = 10
  operations_per_thread = 1000

  def worker(thread_id):
    for i in range(operations_per_thread):
      key = (thread_id * 100) + (i % 100)
      cache.put(key, i)
      cache.get(key)

  threads = []
  for t in range(num_threads):
    thread = threading.Thread(target=worker, args=(t,))
    threads.append(thread)
    thread.start()

  for thread in threads:
    thread.join()

  assert (
      len(cache.cache) <= capacity
  ), f"Stress Test Failed: cache size {len(cache.cache)} exceeds capacity {capacity}"


def run_benchmark():
  capacity = 1000
  operations = 100000

  lru = LRUCache(capacity)
  start_time = time.time()
  for i in range(operations):
    lru.put(i % 2000, i)
    lru.get(i % 2000)
  lru_time = time.time() - start_time

  naive = NaiveLRUCache(capacity)
  start_time = time.time()
  for i in range(operations):
    naive.put(i % 2000, i)
    naive.get(i % 2000)
  naive_time = time.time() - start_time

  print(f"O(1) LRU Execution Time: {lru_time:.4f} seconds")
  print(f"Naive LRU Execution Time: {naive_time:.4f} seconds")


if __name__ == "__main__":
  run_unit_tests()
  run_stress_test()
  run_benchmark()
  print("All unit tests, stress tests, and benchmarks completed successfully!")