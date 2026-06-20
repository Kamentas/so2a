CC = gcc
CFLAGS = -std=gnu23 -O3 -ffast-math -Wall -Wextra -Wpedantic -D_GNU_SOURCE
TARGET = so2a
SRCS = src/main.c \
       src/elf/aarch64/parser.c \
       src/converter/section_builder.c \
       src/converter/symbol_table.c \
       src/converter/relocator.c \
       src/converter/so2a.c \
       src/writer/elf_writer.c \
       src/writer/ar_writer.c
       
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
