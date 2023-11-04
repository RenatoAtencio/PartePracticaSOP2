CXX = g++
CXXFLAGS = -std=c++17 -O3 -Wall -fopenmp

SRC_DIR = src
OBJ_DIR = obj

# Lista de archivos fuente en src/
SRCS = $(wildcard $(SRC_DIR)/*.cpp)

# Lista de archivos objeto generados a partir de los archivos fuente
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# Nombre de los programas ejecutables
TARGET1 = searcher  # Primer programa
TARGET2 = memcache  # Segundo programa
TARGET3 = invertedindex  # Tercer progrma

all: $(TARGET1) $(TARGET2) $(TARGET3)

$(TARGET1): $(OBJ_DIR)/searcher.o
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TARGET2): $(OBJ_DIR)/memcache.o
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TARGET3): $(OBJ_DIR)/invertedindex.o
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -rf $(TARGET1) $(TARGET2) $(TARGET3)
