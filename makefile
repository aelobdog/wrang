all:
	$(CC) -o wrang *.c -ggdb

release:
	$(CC) -o wrang *.c -Ofast -s

install:
	mv wrang $(HOME)/.local/bin/wrang
