CC = g++
CFLAGS = -std=c++14 -I. -DTEST -pthread
.PHONY : clean
OBJECTS = mini_test.o mini_test main_test.o main_test \
	PHI_mini_test.o PHI_mini_test PHI_main_test.o PHI_main_test

mini_test : mini_test.o
	$(CC) $(CFLAGS) mini_test.o -o mini_test

mini_test.o : mini_test.cpp
	$(CC) $(CFLAGS) -c mini_test.cpp

main_test : main_test.o
	$(CC) $(CFLAGS) main_test.o -o main_test

main_test.o : main_test.cpp
	$(CC) $(CFLAGS) -c main_test.cpp

PHI_mini_test : PHI_mini_test.o
	$(CC) $(CFLAGS) -DPHI PHI_mini_test.o -o PHI_mini_test

PHI_mini_test.o : mini_test.cpp
	$(CC) $(CFLAGS) -DPHI -c mini_test.cpp -o PHI_mini_test.o

PHI_main_test : PHI_main_test.o
	$(CC) $(CFLAGS) -DPHI PHI_main_test.o -o PHI_main_test

PHI_main_test.o : main_test.cpp
	$(CC) $(CFLAGS) -DPHI -c main_test.cpp

clean:
	-rm $(OBJECTS)
