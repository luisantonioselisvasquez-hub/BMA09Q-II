CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -Wpedantic -g
TARGET  = trie_demo
OBJS    = main.o trie.o radix.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

main.o: main.c trie.h radix.h
	$(CC) $(CFLAGS) -c main.c

trie.o: trie.c trie.h
	$(CC) $(CFLAGS) -c trie.c

radix.o: radix.c radix.h trie.h
	$(CC) $(CFLAGS) -c radix.c

run: $(TARGET)
	./$(TARGET)

valgrind: $(TARGET)
	valgrind --leak-check=full ./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all run valgrind clean
