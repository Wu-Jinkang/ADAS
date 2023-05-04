CC = gcc
# CFLAGS = -Wall -Wextra -Werror
OBJDIR = obj
BINDIR = bin
SRCDIR = src
LOGDIR = log
RUNDIR = run

all: $(BINDIR)/main $(BINDIR)/hmi_output $(BINDIR)/hmi_input

$(BINDIR)/main: $(OBJDIR)/main.o
	$(CC) $(CFLAGS) -o $@ $^

$(BINDIR)/hmi_output: $(OBJDIR)/hmi_output.o
	$(CC) $(CFLAGS) -o $@ $^

$(BINDIR)/hmi_input: $(OBJDIR)/hmi_input.o
	$(CC) $(CFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJDIR)/*.o $(BINDIR)/* $(LOGDIR)/*  $(RUNDIR)/*

.PHONY: all clean