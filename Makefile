C_FILES=$(shell find . -name \*.c -printf '%P\n')
BIN_FILES_BARE=$(C_FILES:.c=)
BIN_FILES=$(addprefix bin/,${BIN_FILES_BARE})

.PHONY: all clean

all: $(BIN_FILES)

clean:
	rm bin/*

bin/%: %.c
	gcc $< -o $@
