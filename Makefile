# Makefile - CampusCompute build
#
# Build:
#   make
# Clean:
#   make clean
# Run:
#   ./campuscompute <command> [args]

CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -O2 -g
UNAME := $(shell uname -s)
LDFLAGS = -lm -lpthread
ifeq ($(UNAME),Linux)
LDFLAGS += -lrt
endif

SRCS = main.c scheduler.c logger.c ipc.c banker.c worker.c limits.c
OBJS = $(SRCS:.c=.o)
TARGET = campuscompute

.PHONY: all clean run help

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c campuscompute.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) activity.log ipc.log

run: $(TARGET)
	./$(TARGET)

help:
	@echo "Targets: all (default), clean, run, help"
