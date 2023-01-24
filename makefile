all:
	$(CC) -o wrang *.c -ggdb

release:
	$(CC) -o wrang *.c -Ofast -s
