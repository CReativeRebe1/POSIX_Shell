CXX = g++
CXXFLAGS = -std=c++17
LDFLAGS = -lreadline
TARGET = myshell

all: $(TARGET)

$(TARGET): main.cpp
	$(CXX) $(CXXFLAGS) -o $(TARGET) main.cpp $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean