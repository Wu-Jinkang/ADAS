CC = gcc
CFLAGS = -Wall -Wextra -Werror
OBJDIR = obj
BINDIR = bin
SRCDIR = src

all: $(BINDIR)/program

$(BINDIR)/program: $(OBJDIR)/main.o
	$(CC) $(CFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJDIR)/*.o $(BINDIR)/program

.PHONY: all clean