#include "header.h"

Token* token_current(TokenList* tklist) {
  if (tklist->iterator_array->iterator_element + tklist->array_offset == tklist->num_elements) {
    return NULL;
  }
  return &(tklist->iterator_array->tokens[tklist->iterator_array->iterator_element]);
}

Token* token_next(TokenList* tklist) {
  if (tklist->iterator_array->iterator_element + tklist->array_offset == tklist->num_elements) {
    return NULL;
  }
  if (tklist->iterator_array->iterator_element == 255) {
    return &(tklist->iterator_array->next->tokens[0]);
  }
  return &(tklist->iterator_array->tokens[tklist->iterator_array->iterator_element + 1]);
}

u8 token_advance(TokenList* tklist) {
  // printf("iterator index @(%3d)\n", tklist->iterator_array->iterator_element);
  if (tklist->iterator_array->iterator_element + tklist->array_offset + 1 == tklist->num_elements) {
    return 0;
  }
  if (tklist->iterator_array->iterator_element == 255) {
    tklist->iterator_array->iterator_element = 0;
    tklist->iterator_array = tklist->iterator_array->next;
    tklist->array_offset = 256;
  } else {
    ++ tklist->iterator_array->iterator_element;
  }
  return 1;
}

u8 safe_token_advance(TokenList* tklist) {
  if (! token_advance(tklist)) {
    fprintf(stderr, "ERROR: (line %ld): parser internal error, `token_advance` must have failes somewhere\n", token_current(tklist)->line);
    return 0;
  }
  return 1;
}

u8 token_expect_next_and_advance(TokenList* tklist, TokenType type) {
  if (token_next(tklist)->type != type) return 0;
  token_advance(tklist);
  return 1;
}

void token_append(TokenList* tklist, char* ptr, int len, TokenType type, u64 line_no) {
  // printf("cli : %3d   |   ", tklist->current_local_index);
  tklist->current_array->tokens[tklist->current_local_index].ptr = ptr;
  tklist->current_array->tokens[tklist->current_local_index].len = len;
  tklist->current_array->tokens[tklist->current_local_index].type = type;
  tklist->current_array->tokens[tklist->current_local_index].line = line_no;
  // token_print(&(tklist->current_array->tokens[tklist->current_local_index]));

  if (tklist->current_local_index == 255) {
    tklist->current_array->next = malloc(sizeof(TokenArray));
    tklist->current_array->tokens[tklist->current_local_index].next = &(tklist->current_array->next->tokens[0]);
    tklist->current_local_index = 0;
    tklist->current_array = tklist->current_array->next;
  } else {
    tklist->current_local_index ++;
    tklist->current_array->tokens[tklist->current_local_index].next = &(tklist->current_array->tokens[tklist->current_local_index + 1]);
  }

  ++tklist->num_elements;
}

char* token_type_enum_to_string(TokenType type) {
  switch(type) {
  case HASH: return "HASH";
  case STAR: return "STAR";
  case FSLASH: return "FSLASH";
  case USCORE: return "USCORE";
  case LSQR: return "LSQR";
  case RSQR: return "RSQR";
  case LPAR: return "LPAR";
  case RPAR: return "RPAR";
  case ATRATE: return "ATRATE";
  case EXCLAM: return "EXCLAM";
  case COLON: return "COLON";
  case SEM: return "SEM";
  case DASH: return "DASH";
  case BTICK: return "BTICK";
  case WORD: return "WORD";
  case NLINE: return "NLINE";
  case SPACE: return "SPACE";
  case PLUS: return "PLUS";
  case NUM_TOKEN_TYPES: return NULL;
  }
}

void token_print(Token *t) {
  if (t->type == WORD) {
    char token_string[20] = {0};
    memcpy(token_string, t->ptr, t->len);
    printf("[ %10s ] [ %5d ] [ %5ld ] %30s\n", token_type_enum_to_string(t->type), t->len, t->line, token_string);
  } else
    printf("[ %10s ] [ %5d ] [ %5ld ]\n", token_type_enum_to_string(t->type), t->len, t->line);
 
}

void token_list_print(TokenList* tklist) {
  u8 status = 1;
  int i = 0;

  while (status) {
    Token* t = token_current(tklist);
    printf("(%3d) ", i++);
    token_print(t);
    status = token_advance(tklist);
  }
}

TokenList* tokenlist_new() {
  TokenList* tklist = malloc(sizeof(TokenList));

  tklist->num_elements = 0;
  tklist->current_local_index = 0;
  tklist->root = malloc(sizeof(TokenArray));
  tklist->current_array = tklist->root;
  tklist->iterator_array = tklist->root;
  tklist->iterator_array->iterator_element = 0;
  tklist->array_offset = 0;

  return tklist;
}

u8 extract_heading_level(char* c) {
  if ((*c) >= '1' && (*c) <= '6') return (*c) - '0';
  return 0;
}
