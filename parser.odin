package main

import "core:fmt"

Node_Kind :: enum {
	Plaintext,
	Heading,
	Line,
	Newline,
	Bold,
	Italic,
	Underline,
	Code,
	Link,
	Image,
	List,
	Url,
	Root,
}

Node :: struct {
	kind:      Node_Kind,
	text:      string,
	level:     int,
	css_class: string,
	depth:     int, // nesting depth of list items; other kinds leave it zero
	language:  string, // highlight language of code spans; empty means plain
	children:  [dynamic]^Node,
	url:       ^Node,
}

Parser :: struct {
	tokens:       []Token,
	position:     int,
	line_started: bool,
}

parse_tokens :: proc(tokens: []Token) -> ^Node {
	parser := Parser{tokens = tokens}
	document := new_node(.Root)
	parse_until(document, &parser, .End)
	nest_lists(document)
	return document
}

// Items parse flat, each remembering its indent depth; this pass
// tucks deeper runs under the item above them.
nest_lists :: proc(node: ^Node) {
	for child in node.children {
		nest_lists(child)
	}
	restructured := make([dynamic]^Node)
	index := 0
	for index < len(node.children) {
		if node.children[index].kind == .List {
			run_end := index
			for run_end < len(node.children) && node.children[run_end].kind == .List {
				run_end += 1
			}
			nest_list_run(&restructured, node.children[index:run_end])
			index = run_end
		} else {
			append(&restructured, node.children[index])
			index += 1
		}
	}
	node.children = restructured
}

nest_list_run :: proc(into: ^[dynamic]^Node, run: []^Node) {
	chain := make([dynamic]^Node)
	for item in run {
		for len(chain) > 0 && chain[len(chain) - 1].depth >= item.depth {
			pop(&chain)
		}
		if len(chain) == 0 {
			append(into, item)
		} else {
			add_child(chain[len(chain) - 1], item)
		}
		append(&chain, item)
	}
}

// Depth of the list marker starting at the given token,
// or -1 when the line holds no list marker.
list_marker_depth :: proc(parser: ^Parser, from: int) -> int {
	index := from
	depth := 0
	for index < len(parser.tokens) && parser.tokens[index].kind == .Space {
		depth += 1
		index += 1
	}
	if index < len(parser.tokens) && parser.tokens[index].kind == .Plus {
		return depth
	}
	return -1
}

new_node :: proc(kind: Node_Kind) -> ^Node {
	node := new(Node)
	node.kind = kind
	if kind == .Link || kind == .Image {
		node.url = new_node(.Url)
	}
	return node
}

add_child :: proc(parent, child: ^Node) {
	if child == nil {
		return
	}
	append(&parent.children, child)
}

token_at_cursor :: proc(parser: ^Parser) -> ^Token {
	if parser.position >= len(parser.tokens) {
		return nil
	}
	return &parser.tokens[parser.position]
}

peek_next_token :: proc(parser: ^Parser) -> ^Token {
	if parser.position + 1 >= len(parser.tokens) {
		return nil
	}
	return &parser.tokens[parser.position + 1]
}

advance_cursor :: proc(parser: ^Parser) -> bool {
	if parser.position + 1 >= len(parser.tokens) {
		return false
	}
	parser.position += 1
	return true
}

advance_safely :: proc(parser: ^Parser) -> bool {
	if advance_cursor(parser) {
		return true
	}
	report_safe_advance_error(parser)
	return false
}

advance_if_next_is :: proc(parser: ^Parser, kind: Token_Kind) -> bool {
	next := peek_next_token(parser)
	if next == nil || next.kind != kind {
		return false
	}
	advance_cursor(parser)
	return true
}

line_at_cursor :: proc(parser: ^Parser) -> int {
	if token := token_at_cursor(parser); token != nil {
		return token.line
	}
	return 0
}

// All three messages below preserve the original C wording exactly, typo included.
report_advance_error :: proc(parser: ^Parser) {
	fmt.eprintfln(
		"ERROR: (line %d): parser internal error, `token_advance` must have failed somewhere",
		line_at_cursor(parser),
	)
}

report_safe_advance_error :: proc(parser: ^Parser) {
	fmt.eprintfln(
		"ERROR: (line %d): parser internal error, `token_advance` must have failes somewhere",
		line_at_cursor(parser),
	)
}

report_expect_error :: proc(parser: ^Parser) {
	fmt.eprintfln(
		"ERROR: (line %d): parser internal error, `token_expect_next_and_advance` must have failed somewhere",
		line_at_cursor(parser),
	)
}

parse_until :: proc(parent: ^Node, parser: ^Parser, terminator: Token_Kind) {
	outer_line_started := parser.line_started
	parser.line_started = false
	defer parser.line_started = outer_line_started
	for {
		token := token_at_cursor(parser)
		if token == nil || token.kind == terminator {
			return
		}
		handle_token(parent, parser, token)
		if !advance_cursor(parser) {
			return
		}
	}
}

parse_words_only :: proc(parent: ^Node, parser: ^Parser, terminator: Token_Kind) {
	for {
		token := token_at_cursor(parser)
		if token == nil || token.kind == terminator {
			return
		}
		token.kind = .Word
		add_child(parent, parse_word(token))
		if !advance_cursor(parser) {
			return
		}
	}
}

handle_token :: proc(parent: ^Node, parser: ^Parser, token: ^Token) {
	for {
		switch token.kind {
		case .Hash:
			if !parser.line_started {
				add_child(parent, parse_heading(parser))
				return
			}
			token.kind = .Word
		case .Plus:
			if !parser.line_started {
				add_child(parent, parse_list_item(parser))
				return
			}
			token.kind = .Word
		case .Star:
			parser.line_started = true
			add_child(parent, parse_wrapped(parser, .Bold, .Star))
			return
		case .Slash:
			parser.line_started = true
			add_child(parent, parse_wrapped(parser, .Italic, .Slash))
			return
		case .Underscore:
			parser.line_started = true
			add_child(parent, parse_wrapped(parser, .Underline, .Underscore))
			return
		case .Backtick:
			parser.line_started = true
			add_child(parent, parse_code(parser))
			return
		case .Left_Bracket,
		     .Right_Bracket,
		     .Left_Paren,
		     .Right_Paren,
		     .Colon,
		     .Semicolon:
			token.kind = .Word
		case .Space:
			if !parser.line_started && list_marker_depth(parser, parser.position) >= 0 {
				add_child(parent, parse_list_item(parser))
				return
			}
			token.kind = .Word
		case .At:
			parser.line_started = true
			if next := peek_next_token(parser); next != nil && next.kind == .Left_Bracket {
				add_child(parent, parse_link(parser))
				return
			}
			token.kind = .Word
		case .Bang:
			parser.line_started = true
			if next := peek_next_token(parser); next != nil && next.kind == .Left_Bracket {
				add_child(parent, parse_image(parser))
				return
			}
			token.kind = .Word
		case .Dash:
			parser.line_started = true
			if handle_dash(parent, parser) {
				return
			}
			token.kind = .Word
		case .Word:
			parser.line_started = true
			add_child(parent, parse_word(token))
			return
		case .Newline:
			if !parser.line_started {
				add_child(parent, new_node(.Newline))
			}
			parser.line_started = false
			return
		case .End:
			return
		}
	}
}

handle_dash :: proc(parent: ^Node, parser: ^Parser) -> bool {
	if next := peek_next_token(parser); next == nil || next.kind != .Dash {
		return false
	}
	if !advance_cursor(parser) {
		report_advance_error(parser)
		return true
	}
	if after := peek_next_token(parser); after != nil && after.kind == .Dash {
		add_child(parent, new_node(.Line))
	} else {
		dashes := new_node(.Plaintext)
		dashes.text = "--"
		add_child(parent, dashes)
	}
	if !advance_cursor(parser) {
		report_advance_error(parser)
	}
	return true
}

parse_word :: proc(token: ^Token) -> ^Node {
	word := new_node(.Plaintext)
	word.text = token.text
	return word
}

parse_wrapped :: proc(parser: ^Parser, kind: Node_Kind, closer: Token_Kind) -> ^Node {
	node := new_node(kind)
	if !advance_cursor(parser) {
		report_advance_error(parser)
		return nil
	}
	parse_until(node, parser, closer)
	return node
}

parse_code :: proc(parser: ^Parser) -> ^Node {
	node := new_node(.Code)
	if !advance_cursor(parser) {
		report_advance_error(parser)
		return nil
	}
	parse_words_only(node, parser, .Backtick)
	split_code_language(node)
	return node
}

parse_heading :: proc(parser: ^Parser) -> ^Node {
	heading := new_node(.Heading)
	heading.level = 1
	if advance_if_next_is(parser, .Word) {
		level_text := token_at_cursor(parser).text
		if len(level_text) != 1 {
			fmt.eprintfln(
				"WARNING: (line %d): a heading's `level` parameter must be a value in the range [1 .. 6], didn't find a value",
				line_at_cursor(parser),
			)
		} else if level := heading_level(level_text[0]); level == 0 {
			fmt.eprintfln(
				"WARNING: (line %d): a heading's `level` parameter must be a value in the range [1 .. 6], found `%d`",
				line_at_cursor(parser),
				level_text[0],
			)
		} else {
			heading.level = level
		}
	} else {
		fmt.eprintfln(
			"WARNING: (line %d): expected a `level` parameter to the heading; eg: `#3`",
			line_at_cursor(parser),
		)
	}
	if !advance_cursor(parser) {
		report_advance_error(parser)
		return nil
	}
	if current := token_at_cursor(parser); current != nil && current.kind == .Space {
		if !advance_safely(parser) {
			return nil
		}
	}
	parse_until(heading, parser, .Newline)
	parser.line_started = false
	return heading
}

heading_level :: proc(character: byte) -> int {
	if character >= '1' && character <= '6' {
		return int(character - '0')
	}
	return 0
}

parse_list_item :: proc(parser: ^Parser) -> ^Node {
	item := new_node(.List)
	for {
		current := token_at_cursor(parser)
		if current == nil || current.kind != .Space {
			break
		}
		item.depth += 1
		// Cannot fail: the caller saw a list marker ahead.
		advance_cursor(parser)
	}
	if !advance_safely(parser) {
		return nil
	}
	if current := token_at_cursor(parser); current != nil && current.kind == .Space {
		if !advance_safely(parser) {
			return nil
		}
	}
	parse_until(item, parser, .Newline)
	parser.line_started = false
	return item
}

parse_link :: proc(parser: ^Parser) -> ^Node {
	link := new_node(.Link)
	if !advance_if_next_is(parser, .Left_Bracket) {
		report_expect_error(parser)
		return nil
	}
	if !advance_safely(parser) {
		return nil
	}
	parse_until(link, parser, .Right_Bracket)
	if !advance_if_next_is(parser, .Left_Paren) {
		fmt.eprintfln(
			"ERROR: (line %d): improper link, missing url",
			line_at_cursor(parser),
		)
		return nil
	}
	if !advance_safely(parser) {
		return nil
	}
	parse_words_only(link.url, parser, .Right_Paren)
	return link
}

parse_image :: proc(parser: ^Parser) -> ^Node {
	image := new_node(.Image)
	if !advance_if_next_is(parser, .Left_Bracket) {
		report_expect_error(parser)
		return nil
	}
	if !advance_safely(parser) {
		return nil
	}
	parse_until(image, parser, .Colon)
	if !advance_if_next_is(parser, .Word) {
		fmt.eprintfln(
			"ERROR: (line %d): images need to be given a `css class`, did not find one",
			line_at_cursor(parser),
		)
		return nil
	}
	image.css_class = token_at_cursor(parser).text
	for {
		next := peek_next_token(parser)
		if next == nil || next.kind == .Right_Bracket {
			break
		}
		if !advance_safely(parser) {
			return nil
		}
	}
	if !advance_safely(parser) {
		return nil
	}
	if !advance_if_next_is(parser, .Left_Paren) {
		fmt.eprintfln(
			"ERROR: (line %d): improper image, missing url",
			line_at_cursor(parser),
		)
		return nil
	}
	if !advance_safely(parser) {
		return nil
	}
	parse_words_only(image.url, parser, .Right_Paren)
	return image
}
