CXX = g++
CXXFLAGS = -std=c++11 -Wall
JSON_INCLUDE = -I json-develop/single_include

programa: programa.cpp
	$(CXX) $(CXXFLAGS) -o programa programa.cpp $(JSON_INCLUDE)

clean:
	rm -f programa

.PHONY: clean
