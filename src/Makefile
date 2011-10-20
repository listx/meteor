CC = gcc
#CC = clang
CFLAGS = -Wall -Wextra -Werror
# CPU-specific optimization
CFLAGS += -march=native
# Use pipe instead of temporary files b/n various stages of compilation
CFLAGS += -pipe
LFLAGS += -lrt -pthread
OBJECTS = main.o io.o hash.o move.o movegen.o position.o random.o test.o thread.o time.o types.o
LIBOBJECTS = io.o hash.o move.o movegen.o position.o random.o test.o thread.o time.o types.o
LIBNAME = libmeteor.so

meteor: $(OBJECTS)
		$(CC) $(CFLAGS) $(OBJECTS) $(LFLAGS) -o meteor

%.o: %.c
		$(CC) $(CFLAGS) -c $<

clean:
		$(RM) *.o libmeteor* meteor

srclist:
	$(shell find -type f -regex ".*\.\(c\|h\)" > srclist)

lib: $(LIBOBJECTS)
	ar rc $(LIBNAME) $(LIBOBJECTS)
	ar s $(LIBNAME)
