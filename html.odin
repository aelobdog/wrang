package main

import "core:fmt"
import "core:strings"

generate_html :: proc(document: ^Node) -> string {
	page := strings.builder_make()
	write_nodes(&page, []^Node{document}, false)
	return strings.to_string(page)
}

WRAPPED_IN_TAGS := #partial [Node_Kind][2]string{
	.Bold      = {"<strong>", "</strong>"},
	.Italic    = {"<em>", "</em>"},
	.Underline = {"<u>", "</u>"},
	.Code      = {"<code>", "</code>"},
}

write_children :: proc(page: ^strings.Builder, node: ^Node) {
	write_nodes(page, node.children[:], false)
}

// Consecutive list items are grouped into one <ul> by the list case below;
// every other kind ignores the grouping flag.
write_nodes :: proc(page: ^strings.Builder, nodes: []^Node, in_list: bool) {
	if len(nodes) == 0 {
		return
	}
	first := nodes[0]
	rest := nodes[1:]
	if in_list && first.kind != .List {
		return
	}
	switch first.kind {
	case .Root:
		strings.write_string(page, "\n<body>\n<div class=\"content\">\n")
		// Skip the synthetic leading newline the lexer always emits.
		write_nodes(page, first.children[1:], false)
		strings.write_string(page, "\n</div>\n</body>\n")
	case .Plaintext:
		strings.write_string(page, first.text)
	case .Heading:
		fmt.sbprintfln(page, "\n<h%d>", first.level)
		write_children(page, first)
		fmt.sbprintfln(page, "\n</h%d>", first.level)
	case .Line:
		strings.write_string(page, "\n<hr>\n")
	case .Newline:
		strings.write_string(
			page,
			"\n<span style=\"display: block; margin-bottom: 1.5em; overflow: hidden\"></span>\n",
		)
	case .Bold, .Italic, .Underline, .Code:
		tags := WRAPPED_IN_TAGS[first.kind]
		strings.write_string(page, tags[0])
		write_children(page, first)
		strings.write_string(page, tags[1])
	case .Link:
		strings.write_string(page, "\n<a href=\"")
		write_children(page, first.url)
		strings.write_string(page, "\">")
		write_children(page, first)
		strings.write_string(page, "</a>\n")
	case .Image:
		strings.write_string(page, "\n<img src=\"")
		write_children(page, first.url)
		strings.write_string(page, "\" class=\"")
		strings.write_string(page, first.css_class)
		strings.write_string(page, "\" alt=\"")
		write_children(page, first)
		strings.write_string(page, "\">\n")
	case .List:
		if !in_list {
			strings.write_string(page, "\n<ul>\n")
		}
		strings.write_string(page, "<li>")
		write_children(page, first)
		strings.write_string(page, "</li>\n")
		write_nodes(page, rest, true)
		if !in_list {
			strings.write_string(page, "</ul>\n")
			after_run := rest
			for len(after_run) > 0 && after_run[0].kind == .List {
				after_run = after_run[1:]
			}
			write_nodes(page, after_run, false)
		}
		return
	case .Url:
		write_children(page, first)
	}
	write_nodes(page, rest, false)
}
