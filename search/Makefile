CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -g
TARGETS = index_builder index_stats searcher
LIB_OBJS = vector.o string.o posting_list.o query_parser.o hashmap.o boolean_index.o boolean_searcher.o

all: $(TARGETS)

index_builder: index_builder.o $(LIB_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

index_stats: index_stats.o $(LIB_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

searcher: searcher.o $(LIB_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGETS) *.o

.PHONY: all clean
