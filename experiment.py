import os
import sys


upper_limit = 1000000
for arg in sys.argv:
    if arg.startswith("upper_limit="):
        upper_limit = int(arg.split("=")[1])

# Ensure workspace is clean
os.system("make clean-all")
os.system("make all")

# Run experiment
buffer_sizes = [10, 20, 30, 40, 50, 100]
num_threads = [1, 5, 10]
cs_lengths = [0, 1e2, 1e4]
combinations = [(i, j, k, q) for i in num_threads for j in num_threads
                for k in buffer_sizes for q in cs_lengths]
# sort combinations by buffer size, producer count, consumer count
combinations = sorted(combinations, key=lambda x: (x[3], x[2], x[0], x[1]))
num_iterations = 0
total_iterations = len(combinations)
percent_done = 0

for num_producers, num_consumers, buffer_size, cs_len in combinations:
    percent_done = (num_iterations / total_iterations) * 100
    formatted_percent = "%.2f" % percent_done
    print("\r                                           ", end="")
    print("\rRunning experiment: {}%".format(formatted_percent), end="")
    os.system("./mutex-exp {} {} {} {} {}".format(buffer_size, num_producers,
                                               num_consumers, upper_limit, cs_len))
    os.system("./spinlock-exp {} {} {} {} {}".format(buffer_size, num_producers,
                                                  num_consumers, upper_limit, cs_len))
    num_iterations += 1

percent_done = (num_iterations / total_iterations) * 100
print("\r                                          ", end="")
print("\rRunning experiment: 100%")
