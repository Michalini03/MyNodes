CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall
TARGET := MyNodes
SRC := src/main.cpp src/disk.cpp src/filesystem.cpp   # include disk.cpp now

.PHONY: all clean run

all: clean $(TARGET)

$(TARGET):
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)
