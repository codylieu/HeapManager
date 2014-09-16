#You can use either a gcc or g++ compiler
#CC = g++
CC = gcc
DEBUGFLAG = -g -DBFLAG
EXECUTABLES = test_basic test_coalesce_order test_coalesce_random test_stress test_stress_time
ARG = 10 100 1000 10000

all: ${EXECUTABLES} test_o1

test: ${EXECUTABLES} test_o1
	for exec in ${EXECUTABLES}; do \
    		./$$exec ; \
	done
	for arg in ${ARG}; do \
			./test_o1 $$arg ; \
	done

debug: CFLAGS += $(DEBUGFLAG)
debug: $(EXECUTABLES)
	for dbg in ${EXECUTABLES}; do \
		gdb ./$$dbg ; \
	done

test_basic: test_basic.c dmm.o
	$(CC) $(CFLAGS) -o test_basic test_basic.c dmm.o
test_coalesce_order: test_coalesce_order.c dmm.o
	$(CC) $(CFLAGS) -o test_coalesce_order test_coalesce_order.c dmm.o
test_coalesce_random: test_coalesce_random.c dmm.o
	$(CC) $(CFLAGS) -o test_coalesce_random test_coalesce_random.c dmm.o
test_stress: test_stress.c dmm.o
	$(CC) $(CFLAGS) -o test_stress test_stress.c dmm.o
test_stress_time: test_stress_time.c dmm.o
	$(CC) $(CFLAGS) -o test_stress_time test_stress_time.c dmm.o
test_o1: test_o1.c dmm.o
	$(CC) $(CFLAGS) -o test_o1 test_o1.c dmm.o
dmm.o: dmm.c
	$(CC) $(CFLAGS) -c dmm.c 
clean:
	rm -f *.o ${EXECUTABLES} test_o1 a.out
