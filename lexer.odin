package main

import "core:strings"

Token_Kind :: enum {
	Hash,
	Star,
	Slash,
	Underscore,
	Left_Bracket,
	Right_Bracket,
	Left_Paren,
	Right_Paren,
	At,
	Bang,
	Colon,
	Semicolon,
	Dash,
	Backtick,
	Word,
	Newline,
	Space,
	Plus,
	End,
}

Token :: struct {
	kind: Token_Kind,
	text: string,
	line: int,
}

Scanner :: struct {
	source:     string,
	tokens:     [dynamic]Token,
	word_start: int,
	position:   int,
	line:       int,
	escaped:    bool,
}

// Carriage returns never reach the scanner: CRLF, CR, and LF all arrive
// as a single newline.
normalize_line_endings :: proc(source: string) -> string {
	normalized := strings.builder_make()
	character := 0
	for character < len(source) {
		if source[character] == '\r' {
			strings.write_byte(&normalized, '\n')
			character += 1
			if character < len(source) && source[character] == '\n' {
				character += 1
			}
		} else {
			strings.write_byte(&normalized, source[character])
			character += 1
		}
	}
	return strings.to_string(normalized)
}

lex_source :: proc(source: string) -> [dynamic]Token {
	scanner := Scanner {
		source = source,
		line   = 1,
	}
	append(&scanner.tokens, Token{kind = .Newline, text = "\n"})
	for scanner.position < len(scanner.source) {
		step_scanner(&scanner)
	}
	flush_word(&scanner)
	append(&scanner.tokens, Token{kind = .Newline})
	append(&scanner.tokens, Token{kind = .End})
	return scanner.tokens
}

step_scanner :: proc(scanner: ^Scanner) {
	current := scanner.source[scanner.position]
	switch current {
	case '\n':
		flush_word(scanner)
		emit_token(scanner, .Newline, scanner.position, scanner.position + 1)
		scanner.line += 1
		scanner.word_start += 1
		scanner.position += 1
	case ' ':
		flush_word(scanner)
		emit_token(scanner, .Space, scanner.position, scanner.position + 1)
		scanner.word_start += 1
		scanner.position += 1
	case '\\':
		flush_word(scanner)
		scanner.escaped = true
		scanner.word_start += 1
		scanner.position += 1
	case:
		if kind, is_special := special_kind(current); is_special {
			flush_word(scanner)
			// A backslash forces the next special character to be plain text.
			// The flag survives spaces and newlines, matching the C lexer.
			if scanner.escaped {
				emit_token(scanner, .Word, scanner.position, scanner.position + 1)
				scanner.escaped = false
			} else {
				emit_token(scanner, kind, scanner.position, scanner.position + 1)
			}
			scanner.word_start += 1
			scanner.position += 1
		} else {
			scanner.position += 1
		}
	}
}

flush_word :: proc(scanner: ^Scanner) {
	if scanner.word_start != scanner.position {
		emit_token(scanner, .Word, scanner.word_start, scanner.position)
		scanner.word_start = scanner.position
	}
}

emit_token :: proc(scanner: ^Scanner, kind: Token_Kind, from, to: int) {
	append(
		&scanner.tokens,
		Token{kind = kind, text = scanner.source[from:to], line = scanner.line},
	)
}

special_kind :: proc(character: byte) -> (Token_Kind, bool) {
	switch character {
	case '#': return .Hash, true
	case '*': return .Star, true
	case '/': return .Slash, true
	case '_': return .Underscore, true
	case '[': return .Left_Bracket, true
	case ']': return .Right_Bracket, true
	case '(': return .Left_Paren, true
	case ')': return .Right_Paren, true
	case '@': return .At, true
	case '!': return .Bang, true
	case ':': return .Colon, true
	case ';': return .Semicolon, true
	case '-': return .Dash, true
	case '`': return .Backtick, true
	case '+': return .Plus, true
	}
	return .Word, false
}
