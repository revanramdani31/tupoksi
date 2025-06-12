CC = gcc
CFLAGS = -Wall -Wextra -g
SRCS = main.c utils.c linkedlist.c stack.c queue.c task.c project.c undo.c batch.c ileio.c menu.c
OBJS = $(SRCS:.c=.o)
TARGET = proyek

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean