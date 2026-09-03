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
	tokens := lex_source(source)
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

@(test)
indented_item_nests_once :: proc(t: ^testing.T) {
	expect_html(
		t,
		"+ one\n  + sub\n+ two\n",
		WRAPPER_OPEN +
		"\n<ul>\n<li>one\n<ul>\n<li>sub</li>\n</ul>\n</li>\n<li>two</li>\n</ul>\n" +
		SPACER +
		WRAPPER_CLOSE,
	)
}

@(test)
nesting_goes_three_deep :: proc(t: ^testing.T) {
	expect_html(
		t,
		"+ a\n  + b\n    + c\n",
		WRAPPER_OPEN +
		"\n<ul>\n<li>a\n<ul>\n<li>b\n<ul>\n<li>c</li>\n</ul>\n</li>\n</ul>\n</li>\n</ul>\n" +
		SPACER +
		WRAPPER_CLOSE,
	)
}

@(test)
sibling_subitems_share_one_list :: proc(t: ^testing.T) {
	expect_html(
		t,
		"+ one\n  + a\n  + b\n",
		WRAPPER_OPEN +
		"\n<ul>\n<li>one\n<ul>\n<li>a</li>\n<li>b</li>\n</ul>\n</li>\n</ul>\n" +
		SPACER +
		WRAPPER_CLOSE,
	)
}

@(test)
skipped_depth_attaches_above :: proc(t: ^testing.T) {
	expect_html(
		t,
		"+ a\n    + deep\n",
		WRAPPER_OPEN + "\n<ul>\n<li>a\n<ul>\n<li>deep</li>\n</ul>\n</li>\n</ul>\n" + SPACER + WRAPPER_CLOSE,
	)
}

@(test)
indented_text_without_marker_stays_text :: proc(t: ^testing.T) {
	expect_html(t, "  hello\n", WRAPPER_OPEN + "  hello" + SPACER + WRAPPER_CLOSE)
}

@(test)
c_code_highlights :: proc(t: ^testing.T) {
	expect_html(
		t,
		"`c:int x = 42; // answer`\n",
		WRAPPER_OPEN +
		"<code class=\"language-c\"><span class=\"hl-keyword\">int</span> x = <span class=\"hl-number\">42</span>; <span class=\"hl-comment\">// answer</span></code>" +
		SPACER +
		WRAPPER_CLOSE,
	)
}

@(test)
odin_code_highlights :: proc(t: ^testing.T) {
	expect_html(
		t,
		"`odin:package main`\n",
		WRAPPER_OPEN +
		"<code class=\"language-odin\"><span class=\"hl-keyword\">package</span> main</code>" +
		SPACER +
		WRAPPER_CLOSE,
	)
}

@(test)
julia_code_highlights_strings_comments_numbers :: proc(t: ^testing.T) {
	expect_html(
		t,
		"`julia:x = \"hi\" #= note =# 3.14`\n",
		WRAPPER_OPEN +
		"<code class=\"language-julia\">x = <span class=\"hl-string\">\"hi\"</span> <span class=\"hl-comment\">#= note =#</span> <span class=\"hl-number\">3.14</span></code>" +
		SPACER +
		WRAPPER_CLOSE,
	)
}

@(test)
unknown_language_stays_plain :: proc(t: ^testing.T) {
	expect_html(
		t,
		"`python:x = 1`\n",
		WRAPPER_OPEN + "<code>python:x = 1</code>" + SPACER + WRAPPER_CLOSE,
	)
}

@(test)
colon_without_language_stays_plain :: proc(t: ^testing.T) {
	expect_html(t, "`eg: #1`\n", WRAPPER_OPEN + "<code>eg: #1</code>" + SPACER + WRAPPER_CLOSE)
}

@(test)
plain_code_escapes_html :: proc(t: ^testing.T) {
	expect_html(
		t,
		"`<b> & \"q\"`\n",
		WRAPPER_OPEN + "<code>&lt;b&gt; &amp; \"q\"</code>" + SPACER + WRAPPER_CLOSE,
	)
}

@(test)
multiline_code_highlights :: proc(t: ^testing.T) {
	expect_html(
		t,
		"`c:\nint main() {\n  // greet\n  return 0;\n}`\n",
		WRAPPER_OPEN +
		"<code class=\"language-c\">\n<span class=\"hl-keyword\">int</span> main() {\n  <span class=\"hl-comment\">// greet</span>\n  <span class=\"hl-keyword\">return</span> <span class=\"hl-number\">0</span>;\n}</code>" +
		SPACER +
		WRAPPER_CLOSE,
	)
}
