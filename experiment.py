import os
import sys
import matplotlib.pyplot as plt

upper_limit = 100
for arg in sys.argv:
    if arg.startswith("upper_limit="):
        upper_limit = int(arg.split("=")[1])

# Ensure workspace is clean
os.system("make clean-all")
os.system("make all")

# Run experiment
buffer_sizes = [10, 20, 30, 40, 50, 100]
num_threads = [1, 5, 10, 20]
combinations = zip(num_threads, num_threads, buffer_sizes)

for num_producers, num_consumers, buffer_size in combinations:
    os.system("./mutex-exp {} {} {} {}".format(buffer_size, num_producers,
                                               num_consumers, upper_limit))
    os.system("./spinlock-exp {} {} {} {}".format(buffer_size, num_producers,
                                                  num_consumers, upper_limit))
