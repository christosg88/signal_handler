main: test_signal_handler.cpp Makefile
	g++ -Wall -Wextra -std=c++17 -g -O0 -o main test_signal_handler.cpp -pthread
