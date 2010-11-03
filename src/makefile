CC = gcc
CFLAGS = -Wall -Wextra
# Optimization
CFLAGS += -O3
# CPU-specific optimization
CFLAGS += -march=native
# Use pipe instead of temporary files b/n various stages of compilation
CFLAGS += -pipe
# Debugging symbols
#CFLAGS += -g -O0
OBJECTS = io.o main.o move.o position.o types.o

meteor : $(OBJECTS)
		$(CC) $(CFLAGS) $(OBJECTS) -o meteor

%.o : %.c
		$(CC) $(CFLAGS) -c $<

clean:
		$(RM) *.o meteor
