CXX = g++
CXXFLAGS = -std=c++11 -Wall
JSON_INCLUDE = -I json-develop/single_include

programa: backend.cpp
	$(CXX) $(CXXFLAGS) -o backend backend.cpp $(JSON_INCLUDE)

clean:
	rm -f backend

.PHONY: clean
