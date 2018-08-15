.DEFAULT_GOAL := all

CC      := gcc
RM      := rm -rf
SED     := sed
INSTALL := install

CFLAGS 	    += -Wall -std=c99
CFLAGS 	    += -Wall
CPPFLAGS    += -Isrc
LDFLAGS     +=
TARGET_ARCH +=

PREFIX      ?= /usr/local
BINDIR      := $(PREFIX)/bin
SRCDIR      := src
OBJDIR      := src
TEST_SRCDIR := test
TEST_OBJDIR := test

BIN := cg
SRC := $(SRCDIR)/_cg.c $(SRCDIR)/cg.c
OBJ	:= $(OBJDIR)/_cg.o $(SRCDIR)/cg.o

TEST_BIN := cg_test
TEST_SRC := $(SRCDIR)/_cg.c $(TEST_SRCDIR)/test.c
TEST_OBJ := $(OBJDIR)/_cg.o $(TEST_OBJDIR)/test.o

$(BIN) : $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

$(TEST_BIN) : $(TEST_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

$(SRCDIR)/.o : $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

$(TEST_SRCDIR)/.o : $(TEST_SRCDIR)/%.c
	$(SED) -i '1i #define DEBUG' $(SRCDIR)/cg.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<
	$(SED) -i '/1/d' $(SRCDIR)/cg.h

.PHONY: all
all: $(BIN) $(TEST_BIN)

.PHONY: clean
clean:
	$(RM) $(OBJ) $(TEST_OBJ)

.PHONY: uninstall
uninstall:
	$(RM) $(BIN) $(TEST_BIN)
