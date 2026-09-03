package main

import "core:mem"
import "core:strings"
import "core:testing"

SPACER :: "\n<span style=\"display: block; margin-bottom: 1.5em; overflow: hidden\"></span>\n"
WRAPPER_OPEN :: "\n<body>\n<div class=\"content\">\n"
WRAPPER_CLOSE :: "\n</div>\n</body>\n"

render_source :: proc(source: string, backing := context.allocator) -> string {
	work: mem.Dynamic_Arena
	mem.dynamic_arena_init(&work)
	defer mem.dynamic_arena_destroy(&work)
	context.allocator = mem.dynamic_arena_allocator(&work)
	text := normalize_line_endings(source)
	tokens := lex_source(text)
	document := parse_tokens(tokens[:])
	return strings.clone(generate_html(document), backing)
}

expect_html :: proc(t: ^testing.T, source, expected: string) {
	html := render_source(source)
	defer delete(html)
	testing.expect(t, html == expected, "generated html does not match expected output")
}

@(test)
heading_renders_with_level :: proc(t: ^testing.T) {
	expect_html(t, "#2 Hello World\n", WRAPPER_OPEN + "\n<h2>\nHello World\n</h2>\n" + SPACER + WRAPPER_CLOSE)
}

@(test)
inline_formats_render :: proc(t: ^testing.T) {
	expect_html(
		t,
		"*bold* /slant/ _under_ `code`\n",
		WRAPPER_OPEN +
		"<strong>bold</strong> <em>slant</em> <u>under</u> <code>code</code>" +
		SPACER +
		WRAPPER_CLOSE,
	)
}

@(test)
link_renders :: proc(t: ^testing.T) {
	expect_html(
		t,
		"@[click here](https://example.com/x)\n",
		WRAPPER_OPEN + "\n<a href=\"https://example.com/x\">click here</a>\n" + SPACER + WRAPPER_CLOSE,
	)
}

@(test)
image_renders_with_class :: proc(t: ^testing.T) {
	expect_html(
		t,
		"![A bird:flying](./bird.png)\n",
		WRAPPER_OPEN + "\n<img src=\"./bird.png\" class=\"flying\" alt=\"A bird\">\n" + SPACER + WRAPPER_CLOSE,
	)
}

@(test)
consecutive_items_share_one_list :: proc(t: ^testing.T) {
	expect_html(
		t,
		"+ one\n+ two\n",
		WRAPPER_OPEN + "\n<ul>\n<li>one</li>\n<li>two</li>\n</ul>\n" + SPACER + WRAPPER_CLOSE,
	)
}

@(test)
triple_dash_renders_rule :: proc(t: ^testing.T) {
	expect_html(t, "---\n", WRAPPER_OPEN + "\n<hr>\n" + SPACER + WRAPPER_CLOSE)
}

@(test)
double_dash_stays_text :: proc(t: ^testing.T) {
	expect_html(t, "--\n", WRAPPER_OPEN + "--" + WRAPPER_CLOSE)
}

@(test)
escaped_marker_stays_text :: proc(t: ^testing.T) {
	expect_html(t, "\\#hi\n", WRAPPER_OPEN + "#hi" + SPACER + WRAPPER_CLOSE)
}

@(test)
crlf_renders_like_lf :: proc(t: ^testing.T) {
	lf := render_source("#1 Hi\n\n+ a\n+ b\n")
	defer delete(lf)
	crlf := render_source("#1 Hi\r\n\r\n+ a\r\n+ b\r\n")
	defer delete(crlf)
	testing.expect(t, lf == crlf, "crlf input should render exactly like lf input")
	testing.expect(
		t,
		!strings.contains_rune(crlf, '\r'),
		"no carriage returns should survive into the output",
	)
}

@(test)
lone_cr_breaks_lines :: proc(t: ^testing.T) {
	expect_html(t, "a\rb\n", WRAPPER_OPEN + "ab" + SPACER + WRAPPER_CLOSE)
}
