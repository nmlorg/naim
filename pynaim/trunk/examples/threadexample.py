import threading
import time

def f():
  while True:
    time.sleep(1)
    print '%i' % time.time()

t = threading.Thread(None, f)
t.start()
