/*	-----------------------------------------------------------------------------
 *	WRANG, the writing language
 *	-----------------------------------------------------------------------------
 *
 *	MIT License
 *
 *	Copyright (c) 2022 Ashwin Godbole
 *	
 *	Permission is hereby granted, free of charge, to any person obtaining a copy
 *	of this software and associated documentation files (the "Software"), to deal
 *	in the Software without restriction, including without limitation the rights
 *	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *	copies of the Software, and to permit persons to whom the Software is
 *	furnished to do so, subject to the following conditions:
 *
 *	The above copyright notice and this permission notice shall be included in all
 *	copies or substantial portions of the Software.
 *
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *	SOFTWARE.
 *	-----------------------------------------------------------------------------
 */

#include "header.h"

void add_prev_token_as_word(TokenList* tklist, char** tok_start_ptr, char** data_ptr, u64 line) {
  if (*tok_start_ptr != *data_ptr) {
    token_append(tklist, *tok_start_ptr, (int)(*data_ptr - *tok_start_ptr), WORD, line);
    *tok_start_ptr = *data_ptr;
  }
}

// tokenizes file and stores tokens in the tokenlist passed
void WRANG_lex(char* data, TokenList* tklist) {
  token_append(tklist, "\n", 1, NLINE, 0);
  char* data_iterator = data;
  char* tok_start = data;
  u64 line_no = 1;

  u8 escaped = 0;

  while (*data_iterator != '\0') {
    switch(*data_iterator) {
    case '#':
      add_prev_token_as_word(tklist, &tok_start, &data_iterator, line_no);
      if (escaped) {
	token_append(tklist, tok_start, 1, WORD, line_no);
	escaped = 0;
      }
      else token_append(tklist, tok_start, 1, HASH, line_no);
      ++tok_start;
      ++data_iterator;
      break;

    case '*':
      add_prev_token_as_word(tklist, &tok_start, &data_iterator, line_no);
      if (escaped) {
	token_append(tklist, tok_start, 1, WORD, line_no);
	escaped = 0;
      }
      else token_append(tklist, tok_start, 1, STAR, line_no);
      ++tok_start;
      ++data_iterator;
      break;

    case '/':
      add_prev_token_as_word(tklist, &tok_start, &data_iterator, line_no);
      if (escaped) {
	token_append(tklist, tok_start, 1, WORD, line_no);
	escaped = 0;
      }
      else token_append(tklist, tok_start, 1, FSLASH, line_no);
      ++tok_start;
      ++data_iterator;
      break;

    case '_':
      add_prev_token_as_word(tklist, &tok_start, &data_iterator, line_no);
      if (escaped) {
	token_append(tklist, tok_start, 1, WORD, line_no);
	escaped = 0;
      }
      else token_append(tklist, tok_start, 1, USCORE, line_no);
      ++tok_start;
      ++data_iterator;
      break;

    case '\\':
      add_prev_token_as_word(tklist, &tok_start, &data_iterator, line_no);
      // token_append(tklist, tok_start, 1, BSLASH, line_no);
      escaped = 1;
      ++tok_start;
      ++data_iterator;
      break;

    case '[':
      add_prev_token_as_word(tklist, &tok_start, &data_iterator, line_no);
      if (escaped) {
	token_append(tklist, tok_start, 1, WORD, line_no);
	escaped = 0;
      }
      else token_append(tklist, tok_start, 1, LSQR, line_no);
      ++tok_start;
      ++data_iterator;
      break;

    case ']':
      add_prev_token_as_word(tklist, &tok_start, &data_iterator, line_no);
      if (escaped) {
	token_append(tklist, tok_start, 1, WORD, line_no);
	escaped = 0;
      }
      else token_append(tklist, tok_start, 1, RSQR, line_no);
      ++tok_start;
      ++data_iterator;
      break;

    case '(':
      add_prev_token_as_word(tklist, &tok_start, &data_iterator, line_no);
      if (escaped) {
	token_append(tklist, tok_start, 1, WORD, line_no);
	escaped = 0;
      }
      else token_append(tklist, tok_start, 1, LPAR, line_no);
      ++tok_start;
      ++data_iterator;
      break;

    case ')':
      add_prev_token_as_word(tklist, &tok_start, &data_iterator, line_no);
      if (escaped) {
	token_append(tklist, tok_start, 1, WORD, line_no);
	escaped = 0;
      }
      else token_append(tklist, tok_start, 1, RPAR, line_no);
      ++tok_start;
      ++data_iterator;
      break;

    case '-':
      add_prev_token_as_word(tklist, &tok_start, &data_iterator, line_no);
      if (escaped) {
	token_append(tklist, tok_start, 1, WORD, line_no);
	escaped = 0;
      }
      else token_append(tklist, tok_start, 1, DASH, line_no);
      ++tok_start;
      ++data_iterator;
      break;

    case '@':
      add_prev_token_as_word(tklist, &tok_start, &data_iterator, line_no);
      if (escaped) {
	token_append(tklist, tok_start, 1, WORD, line_no);
	escaped = 0;
      }
      else token_append(tklist, tok_start, 1, ATRATE, line_no);
      ++tok_start;
      ++data_iterator;
      break;

    case '!':
      add_prev_token_as_word(tklist, &tok_start, &data_iterator, line_no);
      if (escaped) {
	token_append(tklist, tok_start, 1, WORD, line_no);
	escaped = 0;
      }
      else token_append(tklist, tok_start, 1, EXCLAM, line_no);
      ++tok_start;
      ++data_iterator;
      break;

    case ';':
      add_prev_token_as_word(tklist, &tok_start, &data_iterator, line_no);
      if (escaped) {
	token_append(tklist, tok_start, 1, WORD, line_no);
	escaped = 0;
      }
      else token_append(tklist, tok_start, 1, SEM, line_no);
      ++tok_start;
      ++data_iterator;
      break;

    case '`':
      add_prev_token_as_word(tklist, &tok_start, &data_iterator, line_no);
      if (escaped) {
	token_append(tklist, tok_start, 1, WORD, line_no);
	escaped = 0;
      }
      else token_append(tklist, tok_start, 1, BTICK, line_no);
      ++tok_start;
      ++data_iterator;
      break;

    case '\n':
      add_prev_token_as_word(tklist, &tok_start, &data_iterator, line_no);
      token_append(tklist, tok_start, 1, NLINE, line_no);
      ++line_no;
      ++tok_start;
      ++data_iterator;
      break;

    case ':':
      add_prev_token_as_word(tklist, &tok_start, &data_iterator, line_no);
      if (escaped) {
	token_append(tklist, tok_start, 1, WORD, line_no);
	escaped = 0;
      }
      else token_append(tklist, tok_start, 1, COLON, line_no);
      ++tok_start;
      ++data_iterator;
      break;
      
    case '+':
      add_prev_token_as_word(tklist, &tok_start, &data_iterator, line_no);
      if (escaped) {
	token_append(tklist, tok_start, 1, WORD, line_no);
	escaped = 0;
      }
      else token_append(tklist, tok_start, 1, PLUS, line_no);
      ++tok_start;
      ++data_iterator;
      break;
			
    case ' ':
      add_prev_token_as_word(tklist, &tok_start, &data_iterator, line_no);
      token_append(tklist, tok_start, 1, SPACE, line_no);
      ++tok_start;
      ++data_iterator;
      break;

    default:
      data_iterator++;
    }

    if (*data_iterator == '\0') break;
  }
  add_prev_token_as_word(tklist, &tok_start, &data_iterator, line_no);
  token_append(tklist, NULL, 0, NLINE, 0);
  token_append(tklist, NULL, 0, NUM_TOKEN_TYPES, 0);
}
