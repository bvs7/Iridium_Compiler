CC=gcc
BUILD_DIR=bin/
SRC_DIR=src/
TEST_DIR=tests/
TESTS=test

$(BUILD_DIR)%.o: $(SRC_DIR)%.c
	$(CC) -o $@ -c $<

test: $(BUILD_DIR)test_NFA.o $(BUILD_DIR)NFA.o
	$(CC) -o $(TEST_DIR)test_NFA -c $^