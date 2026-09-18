CC = gcc
CFLAGS = -O3 -fopenmp
LDFLAGS = -fopenmp -lm

TARGET = parallel_pi
SRC = main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
