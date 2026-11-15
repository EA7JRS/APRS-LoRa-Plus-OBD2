# Copyright (C) 2026 Manuel - EA7JRS — GPL-3.0-or-later
CXX ?= g++
CXXFLAGS ?= -std=gnu++17 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined -I../lib/ObdCore/src

run: test_core
	./test_core

test_core: test_core.cpp ../lib/ObdCore/src/obd.h ../lib/ObdCore/src/n2k_engine.h ../lib/ObdCore/src/j1939.h
	$(CXX) $(CXXFLAGS) -o test_core test_core.cpp

clean:
	rm -f test_core
