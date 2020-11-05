CC := gcc 
CXX := g++ 

OBJS := $(patsubst %.c, %.o, $(wildcard ./src/*.c))
OBJS += bench.o
INCLUDE_DIR := -I./src
#.PHONY:all
TARGET := bench_slog
all:$(TARGET)

LDFLAGS = -pthread -lprofiler
CFLAGS = -g
CXXFLAGS = -std=c++17

$(TARGET):$(OBJS)
	$(CXX) $(notdir $^)  $(LDFLAGS) $(CV_LIB) -o $(TARGET)

%.o:%.cc
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIR) -c $< -o $(notdir $@)
%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDE_DIR) -c $< -o $(notdir $@)

.PHONY:clean
clean:
	@rm -f *.o
	@rm -f $(TARGET)
