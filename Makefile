CC = gcc 
CFLAGS = -Wvla -Wextra -Werror -D_GNU_SOURCE
LDFLAGS = -lm

# List of executables to be built
EXECUTABLES = master atomo attivatore alimentazione inibitore

# Default target
all: $(EXECUTABLES)

# Rules for building each component
master: master.o
	$(CC) $(CFLAGS) -o $@ $^

atomo: atomo.o
	$(CC) $(CFLAGS) -o $@ $^

attivatore: attivatore.o
	$(CC) $(CFLAGS) -o $@ $^

alimentazione: alimentazione.o
	$(CC) $(CFLAGS) -o $@ $^

inibitore: inibitore.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Generic rule for compiling source files to object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up compiled files and executables
clean:
	rm -f *.o $(EXECUTABLES)

# Run the program
run: all
	./master

.PHONY: all clean run	# Declare the targets that are not files
