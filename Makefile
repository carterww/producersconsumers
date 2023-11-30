.SILENT:
CS = gcc -Wall main.c helpers.c

all: spinlock mutex spinlock-exp mutex-exp

spinlock:
	$(CS) -D SPINLOCK -o spinlock

mutex:
	$(CS) -D MUTEX -o mutex

spinlock-exp:
	$(CS) -D SPINLOCK -D EXPERIMENTAL -o spinlock-exp

mutex-exp:
	$(CS) -D MUTEX -D EXPERIMENTAL -o mutex-exp

clean:
	rm -f a.out
	rm -f spinlock
	rm -f mutex
	rm -f spinlock-exp
	rm -f mutex-exp

clean-all: clean
	rm -f *.csv
