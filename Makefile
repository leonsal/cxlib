
ASAN=-fsanitize=address,leak
CC=gcc -g -O0 

SOURCES=run_tests.c\
		cx_alloc.c\
		cx_alloc_block.c\
		cx_array.c\
		cx_hmap.c\
		cx_tests.c

HEADERS=cx_alloc.h\
		cx_alloc_block.h\
		cx_array.h\
		cx_hmap.h\
		cx_tests.h

TARGET=run_tests

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS) Makefile
	$(CC) $(SOURCES) -o $(TARGET)

asan: $(SOURCES) $(HEADERS)
	$(CC) $(ASAN) $(SOURCES) -o $(TARGET)

pp:
	gcc -P -E run_tests.c | clang-format --style=LLVM

clean:
	rm $(TARGET)

d: $(TARGET)
	gdb tests

