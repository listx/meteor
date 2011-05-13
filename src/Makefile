CC = gcc
#CC = clang
CFLAGS = -Wall -Wextra
# Optimization
CFLAGS += -O3
# CPU-specific optimization
CFLAGS += -march=native
# Use pipe instead of temporary files b/n various stages of compilation
CFLAGS += -pipe
# Debugging symbols
#CFLAGS += -g -O0 -DNDEBUG
LFLAGS += -lrt -pthread
OBJECTS = io.o main.o move.o movegen.o position.o random.o test.o thread.o time.o types.o

meteor : $(OBJECTS)
		$(CC) $(CFLAGS) $(OBJECTS) $(LFLAGS) -o meteor

%.o : %.c
		$(CC) $(CFLAGS) -c $<

clean:
		$(RM) *.o meteor

srclist:
	$(shell find -type f -regex ".*\.\(c\|h\)" > srclist)
