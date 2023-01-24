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

#ifndef HEADER_H
#define HEADER_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

typedef struct Token Token;
typedef struct TokenArray TokenArray;
typedef struct TokenList TokenList;
typedef struct TreeNode TreeNode;

typedef struct Heading Heading;
typedef struct Bold Bold;
typedef struct Italics Italics;
typedef struct Underline Underline;
typedef struct Link Link;
typedef struct Image Image;
typedef struct Newline Newline;
typedef struct Word Word;

typedef uint8_t  u8;
typedef uint64_t u64;

typedef enum TokenType {
  HASH,    //   #
  STAR,    //   *
  FSLASH,  //   /
  USCORE,  //   _
  LSQR,    //   [
  RSQR,    //   ]
  LPAR,    //   (
  RPAR,    //   )
  ATRATE,  //   @
  EXCLAM,  //   !
  COLON,   //   :
  SEM,     //   ;
  DASH,    //   -
  BTICK,   //   `
  WORD,    //   <text>
  NLINE,   //   <ret>
  SPACE,   //   ' '
  PLUS,    //   +
  NUM_TOKEN_TYPES
} TokenType;

struct Token {
  char* ptr;
  u8 len;
  TokenType type;
  Token* next;
  u64 line;
};

struct TokenArray {
  u8 iterator_element;
  Token	tokens[256];
  TokenArray* next;
};

struct TokenList {
  u8 current_local_index;
  TokenArray* root;
  TokenArray* current_array;
  TokenArray* iterator_array;
  u64 array_offset;
  u64 num_elements;
};

typedef enum ParseTreeNodeType {
  PLAINTEXT,
  HEADING,
  LINE,
  NEWLINE,
  BOLD,
  ITALICS,
  UNDERLINE,
  CODE,
  LINK,
  IMAGE,
  LIST,
  URL,
  ROOT,
} ParseTreeNodeType;

typedef union ParseTreeContentNode {
  Word* word;
  Heading* heading;
  Link* link;
  Image* image;
} ParseTreeContentNode;

struct TreeNode {
  ParseTreeNodeType    node_type;
  ParseTreeContentNode content;
  TreeNode*            right_sibling;
  TreeNode*            left_child;
};

struct Heading {
  u8 level;
};

struct Word {
  Token* token;
};

struct Link {
  TreeNode* url;
};

struct Image {
  TreeNode* url;
  Token* css_class;
};

TokenList* tokenlist_new();
TreeNode*  treenode_new();

Token* token_current(TokenList*);
Token* token_next(TokenList*);
u8     token_advance(TokenList*);
void   token_append(TokenList*, char*, int, TokenType, u64);
void   token_list_print(TokenList*);
void   token_print(Token*);
u8     token_expect_next_and_advance(TokenList*, TokenType);
u8     safe_token_advance(TokenList*);

void tree_print(TreeNode*, int);
u8 extract_heading_level(char*);

void parse_tokens_till(TreeNode**, TokenList*, TokenType);
void parse_tokens_as_words_only(TreeNode**, TokenList*, TokenType);
TreeNode* parse_heading(TokenList*, u8*);
TreeNode* parse_list(TokenList*, u8*);
TreeNode* parse_work(TokenList*);
TreeNode* parse_bold(TokenList*);
TreeNode* parse_italics(TokenList*);
TreeNode* parse_underline(TokenList*);
TreeNode* parse_code(TokenList*);
TreeNode* parse_link(TokenList*);
TreeNode* parse_image(TokenList*);

void WRANG_lex(char*, TokenList*);
TreeNode* WRANG_parse(TokenList*);
void WRANG_gen(TreeNode*, FILE*, u8);

#endif
