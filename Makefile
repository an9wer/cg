.DEFAULT_GOAL := all

CC      := gcc
RM      := rm -rf
SED     := sed
INSTALL := install

CFLAGS 	    += -Wall -std=c99
CPPFLAGS    += -Isrc
LDFLAGS     +=
TARGET_ARCH +=

PREFIX      := /usr/local
BINDIR      := $(PREFIX)/bin
SRCDIR      := src
TEST_SRCDIR := test

BIN := cg
SRC := $(SRCDIR)/cg.c

TEST_BIN := cg_test
TEST_SRC := $(SRCDIR)/cg.c $(TEST_SRCDIR)/test.c

$(BIN): $(SRC)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ $^

$(TEST_BIN): $(TEST_SRC)
	$(SED) -i '1i #define DEBUG' $(SRCDIR)/cg.h
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ $^
	$(SED) -i '1d' $(SRCDIR)/cg.h

.PHONY: all
all: $(BIN) $(TEST_BIN)

.PHONY: clean
clean:
	$(RM) $(BIN) $(TEST_BIN)

.PHONY: install
install:
	$(INSTALL) -d $(BINDIR)
	$(INSTALL) $(BIN) $(BINDIR)

.PHONY: uninstall
uninstall:
	$(RM) $(BINDIR)/$(BIN)
