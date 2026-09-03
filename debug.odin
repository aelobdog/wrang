package main

import "core:fmt"
import "core:strings"

TOKEN_KIND_NAMES := [Token_Kind]string{
	.Hash          = "HASH",
	.Star          = "STAR",
	.Slash         = "SLASH",
	.Underscore    = "UNDERSCORE",
	.Left_Bracket  = "LEFT_BRACKET",
	.Right_Bracket = "RIGHT_BRACKET",
	.Left_Paren    = "LEFT_PAREN",
	.Right_Paren   = "RIGHT_PAREN",
	.At            = "AT",
	.Bang          = "BANG",
	.Colon         = "COLON",
	.Semicolon     = "SEMICOLON",
	.Dash          = "DASH",
	.Backtick      = "BACKTICK",
	.Word          = "WORD",
	.Newline       = "NEWLINE",
	.Space         = "SPACE",
	.Plus          = "PLUS",
	.End           = "END",
}

print_tokens :: proc(tokens: []Token) {
	for token in tokens {
		fmt.printfln(
			"%s %d %d %s",
			TOKEN_KIND_NAMES[token.kind],
			len(token.text),
			token.line,
			token.text,
		)
	}
}

print_tree :: proc(node: ^Node, level: int) {
	dump := strings.builder_make()
	defer strings.builder_destroy(&dump)
	write_tree(&dump, node, level)
	fmt.print(strings.to_string(dump))
}

write_tree :: proc(dump: ^strings.Builder, node: ^Node, level: int) {
	if node == nil {
		return
	}
	for _ in 0 ..< level {
		strings.write_string(dump, "  ")
	}
	fmt.sbprintf(dump, "%d: ", level)
	switch node.kind {
	case .Root:
		strings.write_string(dump, "ROOT NODE\n")
	case .Heading:
		fmt.sbprintfln(dump, "HEADING <h%d>", node.level)
	case .Plaintext:
		fmt.sbprintfln(dump, "WORD: %s (%d)", node.text, len(node.text))
	case .Bold:
		strings.write_string(dump, "BOLD\n")
	case .Italic:
		strings.write_string(dump, "ITALICS\n")
	case .Underline:
		strings.write_string(dump, "UNDERLINE\n")
	case .Code:
		strings.write_string(dump, "CODE\n")
	case .Line:
		strings.write_string(dump, "LINE\n")
	case .Newline:
		strings.write_string(dump, "<br>\n")
	case .Link:
		strings.write_string(dump, "LINK\n")
		write_tree(dump, node.url, level + 1)
	case .Image:
		fmt.sbprintfln(dump, "IMAGE :: (class = '%s')", node.css_class)
		write_tree(dump, node.url, level + 1)
	case .List:
		strings.write_string(dump, "LIST\n")
	case .Url:
		strings.write_string(dump, "URL\n")
	}
	for child in node.children {
		write_tree(dump, child, level + 1)
	}
}
