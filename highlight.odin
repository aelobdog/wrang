package main

import "core:fmt"
import "core:strings"

// Best-effort highlighting for code spans tagged like `c:int x = 1`.
// Unknown languages render as plain code. Only double-quoted strings
// are recognized, so apostrophes never start a string by accident.
Code_Language :: struct {
	line_comment: string,
	block_open:   string,
	block_close:  string,
}

language_syntax :: proc(name: string) -> (Code_Language, bool) {
	switch name {
	case "c":
		return {line_comment = "//", block_open = "/*", block_close = "*/"}, true
	case "odin":
		return {line_comment = "//", block_open = "/*", block_close = "*/"}, true
	case "julia":
		return {line_comment = "#", block_open = "#=", block_close = "=#"}, true
	}
	return {}, false
}

is_code_keyword :: proc(language, word: string) -> bool {
	switch language {
	case "c":
		switch word {
		case "auto", "break", "case", "char", "const", "continue", "default",
		     "do", "double", "else", "enum", "extern", "float", "for", "goto",
		     "if", "int", "long", "register", "return", "short", "signed",
		     "sizeof", "static", "struct", "switch", "typedef", "union",
		     "unsigned", "void", "volatile", "while":
			return true
		}
	case "odin":
		switch word {
		case "package", "import", "foreign", "proc", "return", "defer", "if",
		     "else", "when", "for", "in", "do", "switch", "case", "break",
		     "continue", "fallthrough", "struct", "union", "enum", "map",
		     "dynamic", "distinct", "using", "context", "or_return", "or_else",
		     "inline", "no_inline", "true", "false", "nil":
			return true
		}
	case "julia":
		switch word {
		case "function", "end", "if", "else", "elseif", "for", "while",
		     "return", "break", "continue", "global", "local", "let", "do",
		     "try", "catch", "finally", "throw", "struct", "mutable",
		     "abstract", "primitive", "type", "quote", "macro", "module",
		     "using", "import", "export", "const", "true", "false",
		     "nothing", "missing", "begin":
			return true
		}
	}
	return false
}

// A code span starting with `language:` highlights in that language;
// anything else stays plain code.
split_code_language :: proc(node: ^Node) {
	for child, index in node.children {
		if child.text != ":" {
			continue
		}
		language := code_text(node.children[:index])
		if _, known := language_syntax(language); !known {
			return
		}
		node.language = language
		copy(node.children[:], node.children[index + 1:])
		resize(&node.children, len(node.children) - index - 1)
		return
	}
}

code_text :: proc(nodes: []^Node) -> string {
	text := strings.builder_make()
	for node in nodes {
		strings.write_string(&text, node.text)
	}
	return strings.to_string(text)
}

highlight_code :: proc(page: ^strings.Builder, language_name, code: string) {
	syntax, known := language_syntax(language_name)
	if !known {
		write_escaped(page, code)
		return
	}
	index := 0
	for index < len(code) {
		rest := code[index:]
		// Block comments are checked first so `#=` wins over `#`.
		if strings.has_prefix(rest, syntax.block_open) {
			end := len(rest)
			after_open := rest[len(syntax.block_open):]
			if close := strings.index(after_open, syntax.block_close); close >= 0 {
				end = len(syntax.block_open) + close + len(syntax.block_close)
			}
			write_span(page, "hl-comment", rest[:end])
			index += end
		} else if strings.has_prefix(rest, syntax.line_comment) {
			end := len(rest)
			if newline := strings.index_byte(rest, '\n'); newline >= 0 {
				end = newline
			}
			write_span(page, "hl-comment", rest[:end])
			index += end
		} else if rest[0] == '"' {
			end := scan_string(rest)
			write_span(page, "hl-string", rest[:end])
			index += end
		} else if rest[0] >= '0' && rest[0] <= '9' {
			end := 0
			for end < len(rest) && is_number_char(rest[end]) {
				end += 1
			}
			write_span(page, "hl-number", rest[:end])
			index += end
		} else if is_word_start(rest[0]) {
			end := 0
			for end < len(rest) && is_word_char(rest[end]) {
				end += 1
			}
			if word := rest[:end]; is_code_keyword(language_name, word) {
				write_span(page, "hl-keyword", word)
			} else {
				write_escaped(page, word)
			}
			index += end
		} else {
			write_escaped_byte(page, rest[0])
			index += 1
		}
	}
}

// Length of the string starting at text[0], opening quote included;
// the rest of the text when the string never closes.
scan_string :: proc(text: string) -> int {
	end := 1
	for end < len(text) {
		if text[end] == '\\' {
			end += 2
			continue
		}
		if text[end] == '"' {
			return end + 1
		}
		end += 1
	}
	return len(text)
}

is_word_start :: proc(character: byte) -> bool {
	return (character >= 'a' && character <= 'z') ||
		(character >= 'A' && character <= 'Z') ||
		character == '_'
}

is_word_char :: proc(character: byte) -> bool {
	return is_word_start(character) || (character >= '0' && character <= '9')
}

is_number_char :: proc(character: byte) -> bool {
	return is_word_char(character) || character == '.' || character == '\''
}

write_span :: proc(page: ^strings.Builder, class, text: string) {
	fmt.sbprintf(page, "<span class=\"%s\">", class)
	write_escaped(page, text)
	strings.write_string(page, "</span>")
}

write_escaped :: proc(page: ^strings.Builder, text: string) {
	for index := 0; index < len(text); index += 1 {
		write_escaped_byte(page, text[index])
	}
}

write_escaped_byte :: proc(page: ^strings.Builder, character: byte) {
	switch character {
	case '&':
		strings.write_string(page, "&amp;")
	case '<':
		strings.write_string(page, "&lt;")
	case '>':
		strings.write_string(page, "&gt;")
	case:
		strings.write_byte(page, character)
	}
}
