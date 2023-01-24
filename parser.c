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

TreeNode* treenode_new() {
  TreeNode* node = malloc(sizeof(TreeNode));
  node->node_type = ROOT;
  node->right_sibling = NULL;
  node->left_child = NULL;
  return node;
}

void add_child(TreeNode** parent, TreeNode* child) {
  if (*parent == NULL) {
    fprintf(stderr, "ERROR: cannot add nodes to NULL parent\n");
  }
  
  if ((*parent)->left_child == NULL) {
    (*parent)->left_child = child;
  } else {
    TreeNode** ptr_parent_child = &((*parent)->left_child);
    while ((*ptr_parent_child)->right_sibling != NULL) {
      ptr_parent_child = &(*ptr_parent_child)->right_sibling;
    }
    (*ptr_parent_child)->right_sibling = child;
  }
}

void tree_print(TreeNode* root, int level) {
  if (root == NULL) return;

  for (int i=0; i<level; i++) printf("  ");
  printf("%d: ", level);

  switch(root->node_type) {

  case ROOT:
    printf("ROOT NODE\n");
    break;

  case HEADING:
    printf("HEADING <h%d>\n", root->content.heading->level);
    break;

  case PLAINTEXT:
    {
      char word[50] = {0};
      memcpy(word, root->content.word->token->ptr, root->content.word->token->len);
      printf("WORD: %s\n", word);
    }
    break;

  case BOLD:
    printf("BOLD\n");
    break;

  case ITALICS:
    printf("ITALICS\n");
    break;

  case UNDERLINE:
    printf("UNDERLINE\n");
    break;

  case CODE:
    printf("CODE\n");
    break;

  case LINE:
    printf("LINE\n");
    break;

  case NEWLINE:
    printf("<br>\n");
    break;

  case LINK:
    printf("LINK\n");
    tree_print(root->content.link->url, level+1);
    break;

  case IMAGE:
    {
      char token_string[20] = {0};
      memcpy(token_string, root->content.image->css_class->ptr, root->content.image->css_class->len);
      printf("IMAGE :: (class = '%s')\n", token_string);
    }
    tree_print(root->content.image->url, level + 1);
    break;

  case LIST:
    printf("LIST\n");
    break;

  case URL:
    printf("URL\n");
    break;

  default:
    printf("NOT IMPLEMENTED :: %d\n", root->node_type);
  }

  tree_print(root->left_child, level + 1);
  tree_print(root->right_sibling, level);
}

TreeNode* new_node(ParseTreeNodeType type) {
  TreeNode* node = treenode_new();
  node->node_type = type;

  switch (type) {

  case PLAINTEXT:
    node->content.word = (Word*)malloc(sizeof(Word));
    break;

  case HEADING:
    node->content.heading = malloc(sizeof(Heading));
    break;

  case BOLD:
  case ITALICS:
  case UNDERLINE:
  case CODE:
  case URL:
    break;

  case LINE:
    break;

  case LINK:
    node->content.link = malloc(sizeof(Link));
    node->content.link->url = new_node(URL);
    break;

  case IMAGE:
    node->content.image = malloc(sizeof(Image));
    node->content.image->url = new_node(URL);
    break;

  case LIST:
  case NEWLINE:
  case ROOT:
    break;
  }

  return node;
}

TreeNode* parse_heading(TokenList* tklist, u8 *line_started) {
  TreeNode* heading = new_node(HEADING);
  Token* t = NULL;
  u8 level = 0;

  if (!token_expect_next_and_advance(tklist, WORD)) {
    fprintf(stderr, "WARNING: (line %ld): expected a `level` parameter to the heading; eg: `#3`\n", token_current(tklist)->line);
    level = 1;
  } else {
    t = token_current(tklist);
    level = extract_heading_level(t->ptr);

    if (t->len != 1) {
      fprintf(stderr, "WARNING: (line %ld): a heading's `level` parameter must be a value in the range [1 .. 6], didn't find a value\n", t->line);
      level = 1;
    }
    else if (level == 0) {
      fprintf(stderr, "WARNING: (line %ld): a heading's `level` parameter must be a value in the range [1 .. 6], found `%d`\n", t->line, t->ptr[0]);
      level = 1;
    }
  }

  heading->content.heading->level = level;
  if (! token_advance(tklist)) {
    fprintf(stderr, "ERROR: (line %ld): parser internal error, `token_advance` must have failed somewhere\n", token_current(tklist)->line);
    return NULL;
  }

  if (token_current(tklist)->type == SPACE) {
    if (! safe_token_advance(tklist)) return NULL;
  }

  parse_tokens_till(&heading, tklist, NLINE);

  *line_started = 0;
  return heading;
}

TreeNode* parse_list(TokenList* tklist, u8* line_started) {
  TreeNode* list = new_node(LIST);
  if (! safe_token_advance(tklist)) return NULL;

  Token* t = token_current(tklist);
  if (t->len == 1 && t->ptr[0] == ' ')
    if (! safe_token_advance(tklist))
      return NULL;
  
  parse_tokens_till(&list, tklist, NLINE);
  *line_started = 0;
  return list;
}

TreeNode* parse_word(TokenList* tklist) {
  TreeNode* word = new_node(PLAINTEXT);
  word->content.word->token = token_current(tklist);
  return word;
}

TreeNode* parse_bold(TokenList* tklist) {
  TreeNode* bold = new_node(BOLD);
  if (! token_advance(tklist)) {
    fprintf(stderr, "ERROR: (line %ld): parser internal error, `token_advance` must have failed somewhere\n", token_current(tklist)->line);
    return NULL;
  }
  parse_tokens_till(&bold, tklist, STAR);
  return bold;
}

TreeNode* parse_italics(TokenList* tklist) {
  TreeNode* italics = new_node(ITALICS);
  if (! token_advance(tklist)) {
    fprintf(stderr, "ERROR: (line %ld): parser internal error, `token_advance` must have failed somewhere\n", token_current(tklist)->line);
    return NULL;
  }
  parse_tokens_till(&italics, tklist, FSLASH);
  return italics;
}

TreeNode* parse_underline(TokenList* tklist) {
  TreeNode* underline = new_node(UNDERLINE);
  if (! token_advance(tklist)) {
    fprintf(stderr, "ERROR: (line %ld): parser internal error, `token_advance` must have failed somewhere\n", token_current(tklist)->line);
    return NULL;
  }
  parse_tokens_till(&underline, tklist, USCORE);
  return underline;
}

TreeNode* parse_code(TokenList* tklist) {
  TreeNode* code = new_node(CODE);
  if (! token_advance(tklist)) {
    fprintf(stderr, "ERROR: (line %ld): parser internal error, `token_advance` must have failed somewhere\n", token_current(tklist)->line);
    return NULL;
  }
  parse_tokens_till(&code, tklist, BTICK);
  return code;
}

TreeNode* parse_link(TokenList* tklist) {
  TreeNode* link = new_node(LINK);

  if (!token_expect_next_and_advance(tklist, LSQR)) {
    fprintf(stderr, "ERROR: (line %ld): parser internal error, `token_expect_next_and_advance` must have failed somewhere\n", token_current(tklist)->line);
    return NULL;
  }

  if (! safe_token_advance(tklist)) return NULL;

  parse_tokens_till(&link, tklist, RSQR);

  if (!token_expect_next_and_advance(tklist, LPAR)) {
    fprintf(stderr, "ERROR: (line %ld): improper link, missing url\n", token_current(tklist)->line);
    return NULL;
  }

  if (! safe_token_advance(tklist)) return NULL;

  parse_tokens_as_words_only(&(link->content.link->url), tklist, RPAR);

  return link;
}

TreeNode* parse_image(TokenList* tklist) {
  TreeNode* image = new_node(IMAGE);

  if (!token_expect_next_and_advance(tklist, LSQR)) {
    fprintf(stderr, "ERROR: (line %ld): parser internal error, `token_expect_next_and_advance` must have failed somewhere\n", token_current(tklist)->line);
    return NULL;
  }

  if (! safe_token_advance(tklist)) return NULL;
  parse_tokens_till(&image, tklist, COLON);

  if (!token_expect_next_and_advance(tklist, WORD)) {
    fprintf(stderr, "ERROR: (line %ld): images need to be given a `css class`, did not find one\n", token_current(tklist)->line);
    return NULL;
  }

  image->content.image->css_class = token_current(tklist);

  while (token_next(tklist)->type != RSQR) {
    if (! safe_token_advance(tklist)) return NULL;
  }
  if (! safe_token_advance(tklist)) return NULL;

  if (!token_expect_next_and_advance(tklist, LPAR)) {
    fprintf(stderr, "ERROR: (line %ld): improper image, missing url\n", token_current(tklist)->line);
    return NULL;
  }

  if (! safe_token_advance(tklist)) return NULL;
  parse_tokens_as_words_only(&(image->content.image->url), tklist, RPAR);

  return image;
}

void parse_tokens_as_words_only(TreeNode** parent, TokenList* tklist, TokenType type) {
  u8 status = 1;
  while (status) {
    Token* t = token_current(tklist);
    if (t->type == type) return;
    t->type = WORD;
    add_child(parent, parse_word(tklist));
    status = token_advance(tklist);
  }
}

void parse_tokens_till(TreeNode** parent, TokenList* tklist, TokenType type) {
  u8 status = 1;
  u8 line_started = 0;
  while (status) {
    Token* t = token_current(tklist);
    if (t->type == type) return;

  check_token:
    switch(t->type) {

    case HASH:
      if (! line_started) {
	add_child(parent, parse_heading(tklist, &line_started));
      } else {
	t->type = WORD;
	goto check_token;
      }
      break;

    case PLUS:
      if (! line_started) {
	add_child(parent, parse_list(tklist, &line_started));
      } else {
	t->type = WORD;
	goto check_token;
      }
      break;

    case STAR:
      if (! line_started) line_started = 1;
      add_child(parent, parse_bold(tklist));
      break;

    case FSLASH: 
      if (! line_started) line_started = 1;
      add_child(parent, parse_italics(tklist));
      break;

    case USCORE: 
      if (! line_started) line_started = 1;
      add_child(parent, parse_underline(tklist));
      break;

    case BTICK: 
      if (! line_started) line_started = 1;
      add_child(parent, parse_code(tklist));
      break;

    case LSQR:
      t->type = WORD;
      goto check_token;

    case RSQR:
      t->type = WORD;
      goto check_token;

    case LPAR:
      t->type = WORD;
      goto check_token;

    case RPAR:
      t->type = WORD;
      goto check_token;

    case ATRATE:
      if (! line_started) line_started = 1;
      if (token_next(tklist)->type == LSQR)
	add_child(parent, parse_link(tklist));
      else {
	t->type = WORD;
	goto check_token;
      }
      break;

    case EXCLAM:
      if (! line_started) line_started = 1;
      if (token_next(tklist)->type == LSQR)
	add_child(parent, parse_image(tklist));
      else {
	t->type = WORD;
	goto check_token;
      }
      break;
      
    case COLON:
      t->type = WORD;
      goto check_token;

    case SEM:
      t->type = WORD;
      goto check_token;

    case DASH:
      {
	if (! line_started) line_started = 1;

	Token* tok = token_next(tklist);
	if (tok && tok->type == DASH) {
	  if (token_advance(tklist)) {
	    tok = token_next(tklist);
	    if (tok && tok->type == DASH) {
	      TreeNode* line = new_node(LINE);
	      add_child(parent, line);
	      if (! token_advance(tklist))
		fprintf(stderr, "ERROR: (line %ld): parser internal error, `token_advance` must have failed somewhere\n", tok->line);
	    } else {
	      TreeNode* word = new_node(PLAINTEXT);
	      Token* dash_dash = (Token*)malloc(sizeof(Token));
	      dash_dash->line = tok->line;
	      dash_dash->ptr = "--";
	      dash_dash->len = 2;
	      word->content.word->token = dash_dash;
	      add_child(parent, word);
	      if (! token_advance(tklist))
		fprintf(stderr, "ERROR: (line %ld): parser internal error, `token_advance` must have failed somewhere\n", tok->line);
	    }
	  } else {
	    fprintf(stderr, "ERROR: (line %ld): parser internal error, `token_advance` must have failed somewhere\n", tok->line);
	  }
	}
	else {
	  t->type = WORD;
	  goto check_token;
	}
      }
      break;

    case WORD:
      if (! line_started) line_started = 1;
      add_child(parent, parse_word(tklist));
      break;

    case NLINE:
      {
	if (! line_started) {
	  TreeNode* newline = new_node(NEWLINE);
	  add_child(parent, newline);
	}
	line_started = 0;
      }
      break;

    case SPACE:
      t->type = WORD;
      goto check_token;

    case NUM_TOKEN_TYPES: break;
    }

    status = token_advance(tklist);
  }
}

TreeNode* WRANG_parse(TokenList* tklist) {
  TreeNode* root = treenode_new();
  root->node_type = ROOT;
  parse_tokens_till(&root, tklist, NUM_TOKEN_TYPES);
  return root;
}
