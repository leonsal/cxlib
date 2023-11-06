# Configuration
TARGET=run_tests
CFLAGS=-g -Wall -Wno-unused-function
LIBS=
INCS=
.RECIPEPREFIX = >


# Target object Files
OBJS=run_tests.o\
	cx_alloc.o\
 	cx_alloc_block.o\
 	cx_array.o\
 	cx_hmap.o\
 	cx_str.o\
 	cx_tests.o


# Uses clang instead of gcc
ifdef clang
	CC=clang
endif

# Sets release
ifdef release
	CFLAGS+=-O2 -DNDEBUG
endif

# Sets relinfo
ifdef relinfo
	CFLAGS+=-g -O2 -DNDEBUG
endif

# Sets address sanitizer option
ifdef asan 
	CFLAGS+=-fsanitize=address,leak
endif

# Sets memory sanitizer options
ifdef msan 
	CFLAGS+=-fsanitize=memory -fsanitize-memory-track-origins=2
endif

# Sets thread sanitizer option
ifdef tsan
	CFLAGS+=-fsanitize=thread
endif

# Sets profiling optionProfiling with gprof
ifdef gprof
	CFLAGS+=-pg
endif

# Sets clang xray tracer 
ifdef xray
   ifndef clang
      $(error xray is supported only by clang)
   endif
   CFLAGS+=-fxray-instrument
endif

# Optional custom rule
# .c.o:
# 	$(CC) -c $(CFLAGS) $(INCS) -c $<
#
# Default target
.PHONY: all
all: $(TARGET)

# Link executable
$(TARGET): $(OBJS) Makefile
> $(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LIBS)

# Include options and dependency rules
# Must be put after default target rule
include make_deps
include make_options

# Generate dependencies rules file
make_deps:
> $(CC) -MM *.c > make_deps

# Clean target
.PHONY: clean
clean:
> rm -f $(TARGET)
> rm -f *.o
> rm -f gmon.out

# Clean all target
.PHONY: clean
clean_all: clean
> rm -f make_deps
> rm -f make_options

# Generates .gitignore
.PHONY: clean
gitignore:
> echo "*.o" >  .gitignore
> echo ".cache" >> .gitignore
> echo "make_deps" >> .gitignore
> echo "make_options" >> .gitignore
> echo "$(TARGET)" >> .gitignore

# Debug target program
.PHONY: d
d: $(TARGET)
> gdb $(TARGET)

# Runs target program
.PHONY: r
r: $(TARGET)
> ./$(TARGET)

# Run executable with xray tracing enabled.
# The executable must have been built with xray=1
# Will produce an event log file.
xray-run:
> cmake -E env XRAY_OPTIONS="patch_premain=true xray_mode=xray-basic verbose=1" $(exec)

# Converts xray log to google chrome tracing format
xray-evt:
> llvm-xray convert -output-format=trace_event -output=$(xlog).evt -symbolize -sort -instr_map=$(exec) $(xlog)

.PHONY: make_options
save:
> echo "CC=$(CC)\nCFLAGS=$(CFLAGS)" > make_options

# Show help target
help:
> @echo ">make help                   show this message"
> @echo ">make [<options>...<option>] build project with following options:"
> @echo "  clang=1                    use clang compiler (default=gcc))"
> @echo "  release=1                  release build (default=debug))"
> @echo "  relinfo=1                  release build with debug info"
> @echo "  asan=1                     use address sanitizer (address,leak)"
> @echo "  msan=1                     use memory sanitizer"
> @echo "  tsan=1                     use thread sanitizer"
> @echo "  gprof=1                    generates profiler info"
> @echo "  xray=1                     enable clang xray tracer"
> @echo ">make clean                  remove build artifacts"
> @echo ">make clean_all              remove build artifacts make_deps and make_options"
> @echo ">make gitignore              generates '.gitignore' file"
> @echo ">make xray-run exec=<executable>"
> @echo "  run executable generating xray tracer log"
> @echo ">make xray-evt exec=<executable [args]>"
> @echo "  converts xray log google-chrome tracing format"
