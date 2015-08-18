all: polish_calc

%: %.cpp
	g++ -std=c++11 -g $< -o $@

%: %.c
	gcc $< -o $@

