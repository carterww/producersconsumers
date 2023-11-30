import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.scale import LogScale

DATA_DIR = "data/"
SPINLOCK_FILE = DATA_DIR + "results_spinlock.csv"
MUTEX_FILE = DATA_DIR + "results_mutex.csv"

PLOT_DIR = "plots/"

# These values are already sorted by cs_length, buffer size, producer count,
# consumer count if using the experiment.py script
sl_df = pd.read_csv(SPINLOCK_FILE)
m_df = pd.read_csv(MUTEX_FILE)

# Plot samples against time taken
print("Plotting elapsed time...")
fig, ax = plt.subplots()
ax.plot(sl_df.index, sl_df["time_ms"], 'r', label="Spinlock")
ax.plot(m_df.index, m_df["time_ms"], 'b', label="Mutex")
ax.set_xlabel("Sample")
ax.set_ylabel("Elapsed Time (ms)")
ax.set_title("Elapsed Time for each sample (700,000 upper limit)")
ax.legend()
fig.savefig(PLOT_DIR + "elapsedtime.png")

# Plot buffer size against time taken
print("Plotting buffer size...")
# Get Average time taken for each buffer size
buff_m_df = m_df.groupby("buffer_size").mean().reset_index()
buff_sl_df = sl_df.groupby("buffer_size").mean().reset_index()
buff_m_df.sort_values(by="buffer_size", inplace=True, ignore_index=True)
buff_sl_df.sort_values(by="buffer_size", inplace=True, ignore_index=True)
fig, ax = plt.subplots()
ax.plot(buff_m_df["buffer_size"], buff_m_df["time_ms"], 'bo', label="Mutex")
ax.plot(buff_m_df["buffer_size"], buff_m_df["time_ms"], 'b')
ax.plot(buff_sl_df["buffer_size"], buff_sl_df["time_ms"], 'ro', label="Spinlock")
ax.plot(buff_sl_df["buffer_size"], buff_sl_df["time_ms"], 'r')
ax.set_xlabel("Buffer Size")
ax.set_ylabel("Elapsed Time (ms)")
ax.set_title("Elapsed Time for each buffer size (700,000 upper limit)")
ax.legend()
fig.savefig(PLOT_DIR + "buffersize.png")

# Plot number of producers against time taken
print("Plotting number of producers...")
# Get Average time taken for each number of producers
prod_m_df = m_df.groupby("num_producers").mean().reset_index()
prod_sl_df = sl_df.groupby("num_producers").mean().reset_index()
prod_m_df.sort_values(by="num_producers", inplace=True, ignore_index=True)
prod_sl_df.sort_values(by="num_producers", inplace=True, ignore_index=True)
fig, ax = plt.subplots()
ax.plot(prod_m_df["num_producers"], prod_m_df["time_ms"], 'bo', label="Mutex")
ax.plot(prod_m_df["num_producers"], prod_m_df["time_ms"], 'b')
ax.plot(prod_sl_df["num_producers"], prod_sl_df["time_ms"], 'ro', label="Spinlock")
ax.plot(prod_sl_df["num_producers"], prod_sl_df["time_ms"], 'r')
ax.set_xlabel("Number of Producers")
ax.set_ylabel("Elapsed Time (ms)")
ax.set_title("Elapsed Time for each number of producers (700,000 upper limit)")
ax.legend()
fig.savefig(PLOT_DIR + "numproducers.png")

# Plot number of consumers against time taken
print("Plotting number of consumers...")
# Get Average time taken for each number of consumers
cons_m_df = m_df.groupby("num_consumers").mean().reset_index()
cons_sl_df = sl_df.groupby("num_consumers").mean().reset_index()
cons_m_df.sort_values(by="num_consumers", inplace=True, ignore_index=True)
cons_sl_df.sort_values(by="num_consumers", inplace=True, ignore_index=True)
fig, ax = plt.subplots()
ax.plot(cons_m_df["num_consumers"], cons_m_df["time_ms"], 'bo', label="Mutex")
ax.plot(cons_m_df["num_consumers"], cons_m_df["time_ms"], 'b')
ax.plot(cons_sl_df["num_consumers"], cons_sl_df["time_ms"], 'ro', label="Spinlock")
ax.plot(cons_sl_df["num_consumers"], cons_sl_df["time_ms"], 'r')
ax.set_xlabel("Number of Consumers")
ax.set_ylabel("Elapsed Time (ms)")
ax.set_title("Elapsed Time for each number of consumers (700,000 upper limit)")
ax.legend()
fig.savefig(PLOT_DIR + "numconsumers.png")

# Plot critical section length against time taken
print("Plotting critical section length...")
# Get Average time taken for each critical section length
cs_m_df = m_df.groupby("cs_length").mean().reset_index()
cs_sl_df = sl_df.groupby("cs_length").mean().reset_index()
cs_m_df.sort_values(by="cs_length", inplace=True, ignore_index=True)
cs_sl_df.sort_values(by="cs_length", inplace=True, ignore_index=True)
fig, ax = plt.subplots()
ax.plot(cs_m_df["cs_length"], cs_m_df["time_ms"], 'bo', label="Mutex")
ax.plot(cs_m_df["cs_length"], cs_m_df["time_ms"], 'b')
ax.plot(cs_sl_df["cs_length"], cs_sl_df["time_ms"], 'ro', label="Spinlock")
ax.plot(cs_sl_df["cs_length"], cs_sl_df["time_ms"], 'r')
ax.set_xscale('symlog')
ax.set_yscale('symlog')
ax.set_xlabel("Critical Section Length")
ax.set_ylabel("Elapsed Time (ms)")
ax.set_title("Elapsed Time for each critical section length (700,000 upper limit)")
ax.legend()
fig.savefig(PLOT_DIR + "cslen.png")
