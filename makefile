all: odin-release

odin:
	odin.exe build . -out:wrang.exe

odin-release:
	odin.exe build . -out:wrang.exe -o:speed

odin-check:
	odin.exe check .

odin-test:
	odin.exe test .
