# Configuration
TARGET=run_tests
CFLAGS=-g -Wall -Wno-unused-function
LIBS=
INCS=

# Uses clang instead of gcc
ifdef clang
	CC=clang
endif

# Sets address sanitizer option
ifdef asan 
	CFLAGS+=-fsanitize=address,leak
endif

# Sets release
ifdef release
	CFLAGS+=-O2 -DNDEBUG
endif

# Aplication object Files
OBJS=run_tests.o\
	 cx_alloc.o\
	 cx_alloc_block.o\
	 cx_array.o\
	 cx_hmap.o\
  	 cx_str.o\
	 cx_tests.o

# Default target
all: $(TARGET)

# Include dependency rules
include makedeps

# Optional custom rule
# .c.o:
# 	$(CC) -c $(CFLAGS) $(INCS) -c $<

# Executable
$(TARGET): $(OBJS) Makefile
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LIBS)

# Generate dependencies rules file
makedeps:
	$(CC) -MM *.c > makedeps

# Clean target
.PHONY: clean
clean:
	rm -f $(TARGET)
	rm -f *.o
	rm makedeps

# Preprocess and formats target
.PHONY: pp
pp:
	gcc -P -E run_tests.c | clang-format --style=LLVM

# Preprocess and formats cx_tests.c
.PHONY: pptests
pptests:
	gcc -P -E cx_tests.c | clang-format --style=LLVM

# Debug target program
.PHONY: d
d: $(TARGET)
	gdb $(TARGET)

# Runs target program
.PHONY: r
r: $(TARGET)
	./$(TARGET)

