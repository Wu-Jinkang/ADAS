CC = gcc
# CFLAGS = -Wall -Wextra -Werror
DEPS = conn.h
OBJDIR = obj
BINDIR = bin
SRCDIR = src
LOGDIR = log
RUNDIR = run

all: $(BINDIR)/main $(BINDIR)/hmiOutput $(BINDIR)/hmiInput $(BINDIR)/brakeByWire $(BINDIR)/forwardFacingRadar $(BINDIR)/steerByWire $(BINDIR)/throttleControl $(BINDIR)/frontWindshieldCamera

$(BINDIR)/main: $(OBJDIR)/main.o
	$(CC) $(CFLAGS) -o $@ $^

$(BINDIR)/hmiOutput: $(OBJDIR)/hmiOutput.o
	$(CC) $(CFLAGS) -o $@ $^

$(BINDIR)/hmiInput: $(OBJDIR)/hmiInput.o
	$(CC) $(CFLAGS) -o $@ $^

$(BINDIR)/brakeByWire: $(OBJDIR)/brakeByWire.o
	$(CC) $(CFLAGS) -o $@ $^

$(BINDIR)/forwardFacingRadar: $(OBJDIR)/forwardFacingRadar.o
	$(CC) $(CFLAGS) -o $@ $^

$(BINDIR)/steerByWire: $(OBJDIR)/steerByWire.o
	$(CC) $(CFLAGS) -o $@ $^

$(BINDIR)/throttleControl: $(OBJDIR)/throttleControl.o
	$(CC) $(CFLAGS) -o $@ $^

$(BINDIR)/frontWindshieldCamera: $(OBJDIR)/frontWindshieldCamera.o
	$(CC) $(CFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJDIR)/*.o $(BINDIR)/* $(LOGDIR)/*  $(RUNDIR)/*

.PHONY: all clean