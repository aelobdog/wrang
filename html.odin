package main

import "core:fmt"
import "core:strings"

generate_html :: proc(document: ^Node) -> string {
	page := strings.builder_make()
	write_nodes(&page, []^Node{document})
	return strings.to_string(page)
}

WRAPPED_IN_TAGS := #partial [Node_Kind][2]string{
	.Bold      = {"<strong>", "</strong>"},
	.Italic    = {"<em>", "</em>"},
	.Underline = {"<u>", "</u>"},
}

write_nodes :: proc(page: ^strings.Builder, nodes: []^Node) {
	index := 0
	for index < len(nodes) {
		if nodes[index].kind == .List {
			index = write_list_run(page, nodes, index)
		} else {
			write_single_node(page, nodes[index])
			index += 1
		}
	}
}

write_children :: proc(page: ^strings.Builder, node: ^Node) {
	write_nodes(page, node.children[:])
}

// Consecutive list items share one list; a run ends at the first other kind.
write_list_run :: proc(page: ^strings.Builder, nodes: []^Node, from: int) -> int {
	strings.write_string(page, "\n<ul>\n")
	index := from
	for index < len(nodes) && nodes[index].kind == .List {
		write_list_item(page, nodes[index])
		index += 1
	}
	strings.write_string(page, "</ul>\n")
	return index
}

write_list_item :: proc(page: ^strings.Builder, item: ^Node) {
	strings.write_string(page, "<li>")
	write_nodes(page, item.children[:])
	strings.write_string(page, "</li>\n")
}

write_single_node :: proc(page: ^strings.Builder, node: ^Node) {
	switch node.kind {
	case .Root:
		strings.write_string(page, "\n<body>\n<div class=\"content\">\n")
		// Skip the synthetic leading newline the lexer always emits.
		write_nodes(page, node.children[1:])
		strings.write_string(page, "\n</div>\n</body>\n")
	case .Plaintext:
		strings.write_string(page, node.text)
	case .Heading:
		fmt.sbprintfln(page, "\n<h%d>", node.level)
		write_children(page, node)
		fmt.sbprintfln(page, "\n</h%d>", node.level)
	case .Line:
		strings.write_string(page, "\n<hr>\n")
	case .Newline:
		strings.write_string(
			page,
			"\n<span style=\"display: block; margin-bottom: 1.5em; overflow: hidden\"></span>\n",
		)
	case .Bold, .Italic, .Underline:
		tags := WRAPPED_IN_TAGS[node.kind]
		strings.write_string(page, tags[0])
		write_children(page, node)
		strings.write_string(page, tags[1])
	case .Code:
		code := code_text(node.children[:])
		if node.language == "" {
			strings.write_string(page, "<code>")
			write_escaped(page, code)
			strings.write_string(page, "</code>")
		} else {
			fmt.sbprintf(page, "<code class=\"language-%s\">", node.language)
			highlight_code(page, node.language, code)
			strings.write_string(page, "</code>")
		}
	case .Link:
		strings.write_string(page, "\n<a href=\"")
		write_children(page, node.url)
		strings.write_string(page, "\">")
		write_children(page, node)
		strings.write_string(page, "</a>\n")
	case .Image:
		strings.write_string(page, "\n<img src=\"")
		write_children(page, node.url)
		strings.write_string(page, "\" class=\"")
		strings.write_string(page, node.css_class)
		strings.write_string(page, "\" alt=\"")
		write_children(page, node)
		strings.write_string(page, "\">\n")
	case .List:
		write_list_item(page, node)
	case .Url:
		write_children(page, node)
	}
}
