/* 
  =======================================================================
  This file is part of Minimal (mnml) - A *.mini source to C compiler for 
  the Minimal programming language

  Written in 2025 by approx-error

  Minimal is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Minimal is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
  ======================================================================
*/

#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "inc/tokens.h"
#include "inc/retcodes.h"

char *desc_token(MiniTokenName name) {
  switch (name) {
    case MINI_ID: return "Minimal identifier";
    case MINI_CONST_ID: return "Minimal constant identifier";
    case MINI_EXT_ID: return "Minimal external identifier";
    case C_ID: return "C identifier";
    case VOID: return "void type keyword '<>'";
    case INT: return "int type keyword '<#>'";
    case FLOAT: return "float type keyword '<%%>'";
    case STR: return "string type keyword '<\">'";
    case BOOL: return "boolean type keyword '<B>'";
    case STREAM: return "file stream type keyword '<S>'";
    case LIST_T: return "list type keyword '[]'";
    case DICT_T: return "dict type keyword '[:]'";
    case ENUM_T: return "enum type keyword '{E}'";
    case UNION_T: return "union type keyword '{U}'";
    case STRUCT_T: return "struct type keyword '{S}'";
    case CUSTOM_T: return "custom type keyword '<type>'";
    case IF: return "begin if \'??\'"; 
    case ELSE_IF: return "begin else if '|?'";
    case ELSE: return "begin else '|.'";
    case SWITCH: return "begin switch '##'";
    case CASE: return "begin case '#='";
    case LOOP: return "begin loop '@@'";
    case END_MODULE: return "end module '{{{'";
    case END_MAIN: return "end main '<<<'";
    case END_IF: return "end if '~?'";
    case END_SWITCH: return "end switch '~#'";
    case END_LOOP: return "end loop '~@'";
    case END_FUNC: return "end subprogram '~$'";
    case M_IMPORT: return "minimal stdlib import 'M::'";
    case C_IMPORT: return "C import 'C::'";
    case IMPORT: return "minimal import '::'";
    case READ_WRITE: return "read_write operator '!'";
    case REDIRECT: return "redirect operator '->'";
    case CALL: return "call operator '$'";
    case RETURN: return "return '<-'";
    case BREAK: return "break '.'";
    case CONTINUE: return "continue '..'";
    case MODULE: return "begin module '}}}'";
    case MAIN: return "begin main '>>>'";
    case FUNC: return "begin subprogram '$$'";
    case MAIN_DECLARATION: return "main declaration '!~>..<~!'";
    case TRUE: return "true 'T'";
    case FALSE: return "false 'F'";
    case NUL: return "null 'N'";
    case STDIO: return "standard in/out '...'";
    case ARGV: return "argument vector '[..]'";
    case DEFAULT: return "default '_'";
    case LEFT_PAREN: return "left parenthesis '('";
    case RIGHT_PAREN: return "right parenthesis ')'";
    case LEFT_BRACKET: return "left bracket '['";
    case RIGHT_BRACKET: return "right bracket ']'";
    case LEFT_BRACE: return "left brace '{'";
    case RIGHT_BRACE: return "right brace '}'";
    case VERTICAL_BAR: return "vertical bar: '|'";
    case COMMA: return "comma ','";
    case SEMICOLON: return "semicolon ';'";
    case COLON: return "colon ':'";
    case PERIOD: return "dot '.'";
    case PLUS: return "plus '+'";
    case MINUS: return "minus '-'";
    case TIMES: return "mutliplication symbol '*'";
    case DIV: return "division symbol '/'";
    case MOD: return "modulo operator '%%'";
    case POW: return "exponentiation symbol '**'";
    case SQRT: return "sqrt symbol '\\/'";
    case DEREFERENCE: return "dereference operator '^'";
    case ADDRESS: return "address operator '@'";
    case ASSIGN: return "assignment operator ':='";
    case PLUS_ASSIGN: return "plus-assignment operator '+='";
    case MINUS_ASSIGN: return "minus-assignment operator '-='";
    case TIMES_ASSIGN: return "times-assignment operator '*='";
    case DIV_ASSIGN: return "division-assignment operator '/='";
    case MOD_ASSIGN: return "modulo-assignment operator '%%='";
    case INCREMENT: return "increment operator '++'";
    case DECREMENT: return "decrement operator '--'";
    case EQUALS: return "equality operator '='";
    case NOT_EQUAL: return "inequality operator '~='";
    case LESS_THAN: return "less than -operator '<'";
    case GREATER_THAN: return "more than -operator '>'";
    case LESS_EQUAL: return "less than or equal to -operator '<='";
    case GREATER_EQUAL: return "greater than or equal to -operator '>='";
    case AND: return "and-operator '&'";
    case OR: return "or-operator 'V'";
    case NOT: return "not-operator '~'";
    case INT_LITERAL: return "integer literal";
    case FLOAT_LITERAL: return "float literal";
    case STRING_LITERAL: return "string literal";
    case IRRELEVANT: return "irrelevant";
    case NOT_DETERMINED: return "not determined";
    default: return "desc_tokens: Error: token name not recognized";
  }
}


MiniToken *alloc_token(MiniStatus *status) {
  MiniToken *token = malloc(sizeof(MiniToken));
  if (token == NULL) {
    printf("alloc_token: Memory Error: Failed to allocate space for new token\n");
    *status = ALLOCATION_FAIL;
    return NULL;
  }
  *status = SUCCESS; 
  return token;
}

MiniStatus init_token(MiniToken *token, char *string, int tok_category, int tok_name) {
  size_t length = strlen(string) + 1;
  token->string_repr = malloc(length * sizeof(char));
  if (token->string_repr == NULL) {
    printf("init_token: Memory Error: Failed to allocate space for token string representation\n");
    return ALLOCATION_FAIL;
  }
  strcpy(token->string_repr, string);
  token->category=tok_category;
  token->name = tok_name;
  token->next_token = NULL;
  return SUCCESS;
}

void add_token(MiniHeadToken *head_token, MiniToken *new_token) {
  MiniToken *temp = head_token->head;
  if (temp == NULL) {
    head_token->head = new_token;
  } else {
    while (temp->next_token != NULL) {
      temp = temp->next_token;
    }
    temp->next_token = new_token;
  }
  head_token->token_count++;
}

bool last_token(MiniToken *current_token) {
  if (current_token == NULL) {
    return true;
  } else {
    return false;
  }
}

MiniToken *next_token(MiniToken *current_token, MiniStatus *status) {
  MiniToken *next = current_token->next_token;
  if (next == NULL) {
    //printf("DEBUG: next_token: Last token reached\n");
    *status = LAST_TOKEN;
    return NULL;
  }
  *status = SUCCESS;
  return next;
}

void print_tokens(MiniHeadToken *head_token) {
  MiniToken *temp = head_token->head;
  int counter = 0;
  while (temp != NULL) {
    printf("Token %d: %s  Category: %d, Name: %d\n", counter, temp->string_repr, temp->category, temp->name);
    temp = temp->next_token;
    counter++;
  }
  printf("Total token count: %ld\n", head_token->token_count);
}

void free_tokens(MiniHeadToken *head_token) {
  MiniToken *temp = head_token->head;
  while (temp != NULL) {
    MiniToken *next = temp->next_token;
    free(temp->string_repr);
    temp->string_repr = NULL;
    free(temp);
    temp = next;
  }
  head_token->head = NULL;
  head_token->token_count = 0;
}
