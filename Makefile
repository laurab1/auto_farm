CC = g++
CFLAGS = -std=c++14 -I. -DPHI -pthread

mini_test : mini_test.o
	$(CC) $(CFLAGS) mini_test.o -o mini_test

mini_test.o : mini_test.cpp
	$(CC) $(CFLAGS) -c mini_test.cpp

main_test : main_test.o
	$(CC) $(CFLAGS) main_test.o -o main_test

main_test.o : main_test.cpp
	$(CC) $(CFLAGS) -c main_test.cpp
