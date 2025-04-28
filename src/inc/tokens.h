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

#ifndef MINIMAL_TOKEN_H
#define MINIMAL_TOKEN_H

#include <stdbool.h>
#include "retcodes.h"

typedef enum token_categories {
  CATEGORY_UNDETERMINED = -1,
  IDENTIFIER = 0, // Minimal ID, Minimal constant ID, Minimal external ID, C ID 
  TYPE_KW = 10, // Type Keyword (includes custom types)
  BRANCH_KW, // Branch Keyword
  TERM_KW, // Terminating Keyword
  CONTROL_KW, // Control Keyword
  PROGRAM_BLOCK_KW, // Subprogram Keyword
  LITERAL_KW, // Literal Keyword
  PAREN_SEP = 20, // Parenthetical Separator
  PUNCT_SEP, // Punctuational Separator
  BIN_MATH_OP = 30, // Binary Math Operator
  UNA_MATH_OP, // Unary Math Operator
  BIN_ASSIGN_OP, // Binary Assignment Operator
  UNA_ASSIGN_OP, // Unary Assignment Operator
  COMP_OP, // Comparison Operator
  BIN_LOG_OP, // Binary Logical Operator
  UNA_LOG_OP, // Unary Logical Operator
  LITERAL = 40,
  COMMENT = 50,
  WHITESPACE = 60,
  UNCLASSIFIABLE = 70
} MiniTokenCat;

typedef enum token_names {
  TOKEN_UNDETERMINED = -1,
  // IDENTIFIERS
  MINI_ID = 0,
  MINI_CONST_ID,
  MINI_EXT_ID,
  C_ID,
  // KEYWORDS
  // Type categories
  VOID = 1000,
  INT,
  FLOAT,
  STR,
  BOOL,
  STREAM,
  LIST_T,
  DICT_T,
  ENUM_T,
  UNION_T,
  STRUCT_T,
  CUSTOM_T,
  // Branching: Conditionals
  IF = 1100,
  SWITCH,
  // Branching: Loops
  LOOP = 1200,
  //FOR = 1200,
  //WHILE,
  // Block Terminators
  END_MODULE = 1300,
  END_MAIN,
  ELSE_IF,
  ELSE,
  END_IF,
  CASE,
  END_SWITCH,
  END_LOOP,
  END_FUNC,
  // Control: I/O
  M_IMPORT = 1400,
  C_IMPORT,
  IMPORT,
  READ_WRITE,
  REDIRECT,
  // Control: Flow
  CALL = 1500,
  RETURN,
  BREAK,
  CONTINUE,
  // Program Blocks
  MODULE = 1600,
  MAIN,
  FUNC,
  MAIN_DECLARATION,
  // Keyword Literals
  TRUE = 1700,
  FALSE,
  NUL,
  STDIO,
  ARGV,
  DEFAULT,
  // SEPARATORS
  // Parenthetical
  LEFT_PAREN = 2000,
  RIGHT_PAREN,
  LEFT_BRACKET,
  RIGHT_BRACKET,
  LEFT_BRACE,
  RIGHT_BRACE,
  VERTICAL_BAR,
  // Punctuational
  COMMA,
  SEMICOLON,
  COLON,
  PERIOD,
  // OPERATORS
  // Operators: Binary Math
  PLUS = 3000,
  MINUS,
  TIMES,
  DIV,
  MOD,
  POW,
  // BW_AND, BW_OR, BW_XOR, BW_LEFT_SHIFT, BW_RIGHT_SHIFT
  // Operators: Unary Math
  SQRT = 3100,
  DEREFERENCE,
  ADDRESS,
  // Operators: Binary Assignment
  ASSIGN = 3200,
  PLUS_ASSIGN,
  MINUS_ASSIGN,
  TIMES_ASSIGN,
  DIV_ASSIGN,
  MOD_ASSIGN,
  // Operators: Unary Assignment
  INCREMENT = 3300,
  DECREMENT,
  // Operators: Comparison
  EQUALS = 3400,
  NOT_EQUAL,
  LESS_THAN,
  GREATER_THAN,
  LESS_EQUAL,
  GREATER_EQUAL,
  // Operators: Binary Logical
  AND = 3500,
  OR,
  // Operators: Unary Logical
  NOT = 3600,
  // LITERALS
  INT_LITERAL = 4000,
  FLOAT_LITERAL,
  STRING_LITERAL,
  // IRRELEVANT
  IRRELEVANT = 9000,
  // NOT DETERMINED
  NOT_DETERMINED = 10000
} MiniTokenName;

extern const size_t MINIMAL_IDENTIFIER_MAX_LEN;
extern const char *NO_SEMICOLON_AFTER;

extern const char *MINIMAL_VAR_KW_MID_SYMBOLS;
extern const char *MINIMAL_VAR_KW_BEGIN_SYMBOLS;
extern const char *MINIMAL_VAR_KW_END_SYMBOLS;

extern const char *MINIMAL_BRANCH_KWDS[];
extern const char *MINIMAL_TERM_KWDS[];
extern const char *MINIMAL_CONTROL_KWDS[];
extern const char *MINIMAL_PROGRAM_BLOCK_KWDS[];
extern const char *MINIMAL_LITERAL_KWDS[];

extern const char *MINIMAL_PAREN_SEP[];
extern const char *MINIMAL_PUNCT_SEP[];

extern const char *MINIMAL_BIN_MATH_OP[];
extern const char *MINIMAL_UNA_MATH_OP[];
extern const char *MINIMAL_BIN_ASSIGN_OP[];
extern const char *MINIMAL_UNA_ASSIGN_OP[];
extern const char *MINIMAL_COMP_OP[];
extern const char *MINIMAL_BIN_LOG_OP[];
extern const char *MINIMAL_UNA_LOG_OP[];

typedef struct minimal_token_specification {
  char *string_repr;
  MiniTokenCat category;
  MiniTokenName name;
  struct minimal_token_specification *next_token;
} MiniToken;

typedef struct minimal_head_token_spec {
  uint64_t token_count;
  MiniToken *head;
} MiniHeadToken;

// Token functions:
char *desc_token(MiniTokenName name);
MiniToken *alloc_token(MiniStatus *status);
MiniStatus init_token(MiniToken *token, char *string, int category, int name);
void add_token(MiniHeadToken *head_token, MiniToken *new_token);
bool last_token(MiniToken *current_token);
MiniToken *next_token(MiniToken *current_token, MiniStatus *status);
void print_tokens(MiniHeadToken *head_token);
void free_tokens(MiniHeadToken *head_token);

// Lexer functions:
MiniStatus tokenize(char *input_file, char *output_file, MiniHeadToken *head_token, int verbose);

#endif
