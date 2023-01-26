# Wrang

### The Writing Language

Wrang is a successor to Sitefl, the other markup language that I wrote to help generate my webpages.

It is supposed to be simple, and supports only those features that I would use to write my blogs. This includes headings, links, images, bullets and basic text formatting like bold, italics and undeline.

### Getting Started

Using wrang is super simple!

#### Clone the repository
```bash
$ git clone https://github.com/aelobdog/wrang
```

#### Build
```bash
$ make release
```
or
```bash
$ gcc -o wrang *.c -O3 -s
```

#### Run
```bash
$ ./wrang <wrang input> <html output> [ -css stylesheet ] [ -title title ]
```

To make life easier, consider moving the `wrang` binary to some place in your path.
