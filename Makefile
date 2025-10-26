CC=gcc
CFLAGS=-Wall -Wextra -O2
LDFLAGS=-pthread -lrt

SRC_DIR=src
BIN_DIR=bin

PRODUCER=$(BIN_DIR)/producer
CONSUMER=$(BIN_DIR)/consumer
CLEANUP=$(BIN_DIR)/cleanup

all: dirs $(PRODUCER) $(CONSUMER) $(CLEANUP)

dirs:
	mkdir -p $(BIN_DIR)

$(PRODUCER): $(SRC_DIR)/producer.c $(SRC_DIR)/common.h
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

$(CONSUMER): $(SRC_DIR)/consumer.c $(SRC_DIR)/common.h
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

$(CLEANUP): $(SRC_DIR)/cleanup.c $(SRC_DIR)/common.h
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

run-demo: all
	# Demo: produce & consume 20 items each
	./bin/producer 20 & ./bin/consumer 20 &

clean:
	rm -rf $(BIN_DIR)

.PHONY: all dirs run-demo clean
