all:
	$(CC) -o bin/wrang *.c -ggdb

release:
	$(CC) -o bin/wrang *.c -Ofast -s

install:
	mv wrang $(HOME)/.local/bin/wrang
