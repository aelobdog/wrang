package main

import "core:fmt"
import "core:mem"
import "core:os"
import "core:strings"

show_usage :: proc() {
	fmt.println("usage:\n\n  wrang input_file output_file [ --css stylesheet ] [ --title \"title text\" ]\n")
}

main :: proc() {
	arguments := os.args[1:]

	if len(arguments) < 2 {
		show_usage()
		os.exit(1)
	}

	input := arguments[0]
	output := arguments[1]

	// Everything below except the input file lives here
	// and is freed together on exit.
	work: mem.Dynamic_Arena
	mem.dynamic_arena_init(&work)
	defer mem.dynamic_arena_destroy(&work)
	context.allocator = mem.dynamic_arena_allocator(&work)

	header, header_ok := build_header(arguments[2:])
	if !header_ok {
		os.exit(1)
	}

	source, read_error := os.read_entire_file(input, context.allocator)
	// Empty input is rejected, as the original C build did.
	if read_error != nil || len(source) == 0 {
		fmt.eprintln("ERROR: could not read file's contents.")
		os.exit(1)
	}

	tokens := lex_source(string(source))
	document := parse_tokens(tokens[:])

	page := strings.builder_make()
	strings.write_string(
		&page,
		"<html>\n<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n",
	)
	strings.write_string(&page, strings.to_string(header))
	strings.write_string(&page, generate_html(document))
	strings.write_string(&page, "</html>")

	if err := os.write_entire_file(output, strings.to_string(page)); err != nil {
		fmt.eprintln("ERROR: could not write output file.")
		os.exit(1)
	}
	fmt.println("done.")
}

build_header :: proc(options: []string) -> (strings.Builder, bool) {
	header := strings.builder_make()
	if len(options) == 0 {
		return header, true
	}
	strings.write_string(&header, "<head>\n")
	index := 0
	for index < len(options) {
		if options[index] == "-css" {
			if index == len(options) - 1 {
				fmt.println("ERROR: missing path to stylesheet.")
				show_usage()
				return header, false
			}
			fmt.sbprintfln(
				&header,
				"<link rel=\"stylesheet\" href=\"%s\">",
				options[index + 1],
			)
			index += 1
		} else if options[index] == "-title" {
			if index == len(options) - 1 {
				fmt.println("ERROR: missing title.")
				show_usage()
				return header, false
			}
			fmt.sbprintfln(&header, "<title>%s</title>", options[index + 1])
			index += 1
		}
		index += 1
	}
	strings.write_string(&header, "</head>\n")
	return header, true
}
