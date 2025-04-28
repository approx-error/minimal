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
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "inc/retcodes.h"
#include "inc/preprocessor.h"
#include "inc/tokens.h"

// XXX: If need new symbols, maybe use '&' since it only has 1 use at the moment
// Also backticks ` and apostrophes ' are not currently used

// Ideas for single capital letters:
// A: Bitwise and?
// B: Boolean (already implemented)
// C: C-function (already implemented)
// D: 
// E: Enum?
// F: False (already implemented)
// G:
// H:
// I:
// J:
// K:
// L:
// M:
// N: Null (already implemented)
// O:
// P:
// Q:
// R:
// S: file stream (already implemented)
// T: True (already implemented)
// U: Union?
// V:
// W:
// X:
// Y:
// Z:

const size_t MINIMAL_IDENTIFIER_MAX_LEN = 8;

const char *MINIMAL_VAR_KW_MID_SYMBOLS = "#%\"BSEU:,^";
const char *MINIMAL_VAR_KW_BEGIN_SYMBOLS = "<[{";
const char *MINIMAL_VAR_KW_END_SYMBOLS = ">]}";
const char *MINIMAL_BRANCH_KWDS[] = {"??", "##", "@@"};
const char *MINIMAL_TERM_KWDS[] = {"{{{", "<<<", "|?", "|.", "~?", "#=", "~#", "~@", "~$"};
// else if, else and case are terminating keywords and not branch keywords because even though
// they contain a branch point, a more important property of them is that they terminate a previous
// branch point that was started by an if or switch. The fact that them being terminating keywords
// is more important is because this means that they can't exist without a corresponding if or
// switch and thus makes parsing easier
const char *MINIMAL_CONTROL_KWDS[] = {"M::", "C::", "::", "!", "->", "$", "<-", ".", ".."};
const char *MINIMAL_PROGRAM_BLOCK_KWDS[] = {"}}}", ">>>", "$$", "!~>..<~!"};
const char *MINIMAL_LITERAL_KWDS[] = {"T", "F", "N", "...", "[..]", "_"};

const char *MINIMAL_PAREN_SEP[] = {"(", ")", "[", "]", "{", "}", "|"};
const char *MINIMAL_PUNCT_SEP[] = {",", ";", ":"};

const char *MINIMAL_BIN_MATH_OP[] = {"+", "-", "*", "/", "**", "%"};
const char *MINIMAL_UNA_MATH_OP[] = {"\\/", "^", "@"}; // Remember that list indexing is also a
                                                       // unary math operation, it just doesn't have a
                                                       // single token associated with it
//const char *MINIMAL_BIN_STR_OP[] = {"||"}; XXX: Concatenation: Not implemented into the lexer yet
const char *MINIMAL_BIN_ASSIGN_OP[] = {":=", "+=", "-=", "*=", "/=", "%="};
const char *MINIMAL_UNA_ASSIGN_OP[] = {"++", "--"};
const char *MINIMAL_COMP_OP[] = {"=", "~=", "<", ">", "<=", ">="};
const char *MINIMAL_BIN_LOG_OP[] = {"V", "&"};
const char *MINIMAL_UNA_LOG_OP[] = {"~"};


static bool is_comment(char *string) {
  if (strlen(string) < 2) {
    return false;
  }

  char first = string[0];
  char second = string[1];
  if (first == '/' && second == '/') {
    return true;
  } else {
    return false;
  }
}

static bool is_identifier(char *candidate) {
  size_t len = strlen(candidate);
  if (len == 0) {
    return false;
  } else if (isupper(candidate[0])) {
    if (len == 1) {
      return false;
    } else if (len == 2) {
      if (isupper(candidate[1]) || isdigit(candidate[1])) {
        return true;
      } else {
        return false;
      }
    } else {
      if (candidate[0] == 'C' && candidate[1] == ':') {
        for (size_t i = 2; i < len; i++) {
          if (!isdigit(candidate[i]) && !islower(candidate[i]) && candidate[i] != '_') {
            return false;
          }
        }
        return true;
      } else if (len - 2 <= MINIMAL_IDENTIFIER_MAX_LEN && candidate[0] == 'M' && candidate[1] == ':') {
        for (size_t i = 2; i < len; i++) {
          if (!isdigit(candidate[i]) && !islower(candidate[i])) {
            return false;
          }
        }
        return true;
      } else if (len <= MINIMAL_IDENTIFIER_MAX_LEN) {
        for (size_t i = 1; i < len; i++) {
          if (!isupper(candidate[i]) && !isdigit(candidate[i])) {
            return false;
          }
        }
        return true;
      } else {
        return false;
      }
    }
  } else if (islower(candidate[0])) {
    if (len <= MINIMAL_IDENTIFIER_MAX_LEN) {
      for (size_t i = 1; i < len; i++) {
        if (!islower(candidate[i]) && !isdigit(candidate[i])) {
          return false;
        }
      }
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

static bool is_type_keyword(char *candidate) {
  size_t len = strlen(candidate);
  char begin = candidate[0];

  size_t begin_count = strlen(MINIMAL_VAR_KW_BEGIN_SYMBOLS);
  size_t mid_count = strlen(MINIMAL_VAR_KW_MID_SYMBOLS);
  size_t end_count = strlen(MINIMAL_VAR_KW_END_SYMBOLS);
  
  int begin_id = -1;
  for (size_t i = 0; i < begin_count; i++) {
    if (MINIMAL_VAR_KW_BEGIN_SYMBOLS[i] == begin) {
      begin_id = i;
      break;
    }
  }

  if (begin_id == -1) {
    return false;
  }

  int end_position = len - 1;
  for (int i = end_position; i >= 0; i--) {
    if (candidate[i] != '^') {
      end_position = i;
      break;
    }
  }

  if (end_position == 0) {
    return false;
  }
  
  char end = candidate[end_position];
  int end_id = -1;
  for (size_t i = 0; i < end_count; i++) {
    if (MINIMAL_VAR_KW_END_SYMBOLS[i] == end) {
      end_id = i;
      break;
    }
  }

  if (end_id == -1) {
    return false;
  }

  if (begin_id != end_id) {
    return false;
  }
  
  for (int i = 1; i < end_position; i++) {
    bool valid = false;
    for (size_t j = 0; j < mid_count; j++) {
      if (candidate[i] == MINIMAL_VAR_KW_MID_SYMBOLS[j]) {
        valid = true;
      }
    }
    if (!valid) {
      return false;
    }
  }
  return true;
}

static bool is_custom_type_kw(char *candidate) {
  size_t length = strlen(candidate);
  if (length < 3) return false;

  char begin = candidate[0];
  char end = candidate[length - 1];

  if (!(begin == '<' && end == '>')) {
    return false;
  }

  if (!(islower(candidate[1]))) {
    return false;
  }
  if (length == 3) return true;

  for (size_t i = 2; i < length - 1; i++) {
    if (!(islower(candidate[i]) || isdigit(candidate[i]))) {
      return false;
    }
  }

  return true;
}

static bool is_branch_keyword(char *candidate) {
  size_t kwds = sizeof(MINIMAL_BRANCH_KWDS) / sizeof(char *);

  for (size_t i = 0; i < kwds; i++) {
    if (strcmp(candidate, MINIMAL_BRANCH_KWDS[i]) == 0) {
      return true;
    }
  }
  return false;
}

static bool is_terminating_keyword(char *candidate) {
  size_t kwds = sizeof(MINIMAL_TERM_KWDS) / sizeof(char *);

  for (size_t i = 0; i < kwds; i++) {
    if (strcmp(candidate, MINIMAL_TERM_KWDS[i]) == 0) {
      return true;
    }
  }
  return false;
}

static bool is_control_keyword(char *candidate) {
  size_t kwds = sizeof(MINIMAL_CONTROL_KWDS) / sizeof(char *);

  for (size_t i = 0; i < kwds; i++) {
    if (strcmp(candidate, MINIMAL_CONTROL_KWDS[i]) == 0) {
      return true;
    }
  }
  return false;
}

static bool is_program_block_kw(char *candidate) {
  size_t kwds = sizeof(MINIMAL_PROGRAM_BLOCK_KWDS) / sizeof(char *);

  for (size_t i = 0; i < kwds; i++) {
    if (strcmp(candidate, MINIMAL_PROGRAM_BLOCK_KWDS[i]) == 0) {
      return true;
    }
  }
  return false;
}

static bool is_literal_keyword(char *candidate) {
  size_t kwds = sizeof(MINIMAL_LITERAL_KWDS) / sizeof(char *);

  for (size_t i = 0; i < kwds; i++) { 
    if (strcmp(candidate, MINIMAL_LITERAL_KWDS[i]) == 0) {
      return true;
    }
  }
  return false;
}

static bool is_parenthetical_separator(char *candidate) {
  size_t seps = sizeof(MINIMAL_PAREN_SEP) / sizeof(char *);

  for (size_t i = 0; i < seps; i++) {
    if (strcmp(candidate, MINIMAL_PAREN_SEP[i]) == 0) {
      return true;
    }
  }
  return false;
}

static bool is_punctuational_separator(char *candidate) {
  size_t seps = sizeof(MINIMAL_PUNCT_SEP) / sizeof(char *);

  for (size_t i = 0; i < seps; i++) {
    if (strcmp(candidate, MINIMAL_PUNCT_SEP[i]) == 0) {
      return true;
    }
  }
  return false;
}

static bool is_binary_math_operator(char *candidate) {
  size_t ops = sizeof(MINIMAL_BIN_MATH_OP) / sizeof(char *);

  for (size_t i = 0; i < ops; i++) {
    if (strcmp(candidate, MINIMAL_BIN_MATH_OP[i]) == 0) {
      return true;
    }
  }
  return false;
}

static bool is_unary_math_operator(char *candidate) {
  size_t ops = sizeof(MINIMAL_UNA_MATH_OP) / sizeof(char *);

  for (size_t i = 0; i < ops; i++) {
    if (strcmp(candidate, MINIMAL_UNA_MATH_OP[i]) == 0) {
      return true;
    }
  }
  return false;
}

static bool is_binary_assignment_operator(char *candidate) {
  size_t ops = sizeof(MINIMAL_BIN_ASSIGN_OP) / sizeof(char *);

  for (size_t i = 0; i < ops; i++) {
    if (strcmp(candidate, MINIMAL_BIN_ASSIGN_OP[i]) == 0) {
      return true;
    }
  }
  return false;
}

static bool is_unary_assignment_operator(char *candidate) {
  size_t ops = sizeof(MINIMAL_UNA_ASSIGN_OP) / sizeof(char *);

  for (size_t i = 0; i < ops; i++) {
    if (strcmp(candidate, MINIMAL_UNA_ASSIGN_OP[i]) == 0) {
      return true;
    }
  }
  return false;
}

static bool is_comparison_operator(char *candidate) {
  size_t ops = sizeof(MINIMAL_COMP_OP) / sizeof(char *);

  for (size_t i = 0; i < ops; i++) {
    if (strcmp(candidate, MINIMAL_COMP_OP[i]) == 0) {
      return true;
    }
  }
  return false;
}

static bool is_binary_logical_operator(char *candidate) {
  size_t ops = sizeof(MINIMAL_BIN_LOG_OP) / sizeof(char *);

  for (size_t i = 0; i < ops; i++) {
    if (strcmp(candidate, MINIMAL_BIN_LOG_OP[i]) == 0) {
      return true;
    }
  }
  return false;
}

static bool is_unary_logical_operator(char *candidate) {
  size_t ops = sizeof(MINIMAL_UNA_LOG_OP) / sizeof(char *);

  for (size_t i = 0; i < ops; i++) {
    if (strcmp(candidate, MINIMAL_UNA_LOG_OP[i]) == 0) {
      return true;
    }
  }
  return false;
}

static bool is_number(char *candidate) {
  size_t len = strlen(candidate);
  bool decimal_found = false;
  bool exp_found = false;
  size_t i;
  if (candidate[0] == '-') {
    i = 1;
  } else {
    i = 0;
  }
  for (i = i; i < len; i++) {
    if (!isdigit(candidate[i])) {
      switch (candidate[i]) {
        case '.':
          if (decimal_found) {
            return false;
          } else {
            decimal_found = true;
            break;
          }
        case 'e':
          if (exp_found) {
            return false;
          } else {
            exp_found = true;
            break;
          }
        default:
          return false;
      }
    }
  }
  return true;
}


static bool is_string(char *candidate) {
  size_t len = strlen(candidate);
  int quote_counter = 0;
  size_t i;
  if (candidate[0] == '"') {
    quote_counter++;
    i = 1;
  } else {
    i = 0;
  }
  for (i = i; i < len; i++) {
    if (candidate[i] == '"' && candidate[i - 1] != '\\') {
      quote_counter += 1;
    }
  }

  if (quote_counter < 2) {
    return false;
  } else if (quote_counter == 2 && candidate[len - 1] == '"') {
    return true;
  } else {
    return false;
  }
}

static MiniTokenCat categorize_token(char *token) {
  size_t len = strlen(token);
  if (len == 0) {
    return UNCLASSIFIABLE;
  }

  if (len == 1 && isspace(token[0])) {
    return WHITESPACE;
  }

  if (is_comment(token)) {
    return COMMENT;
  }

  if (isalpha(token[0]) || token[0] == '_') {
    if (is_identifier(token)) {
      return IDENTIFIER;
    } else {
      if (token[0] == 'C' || token[0] == 'M') {
        if (is_control_keyword(token)) {
          return CONTROL_KW;
        }
      } else {
        if (is_literal_keyword(token)) {
          return LITERAL_KW;
        }
      }
      return UNCLASSIFIABLE;
    }
  }

  if (isdigit(token[0]) || (token[0] == '-' && isdigit(token[1]))) {
    if (is_number(token)) {
      return LITERAL;
    } else {
      return UNCLASSIFIABLE;
    } 
  }

  if (token[0] == '"') {
    if (is_string(token)) {
      return LITERAL;
    } else {
      return UNCLASSIFIABLE;
    }
  }

  if (is_type_keyword(token) || is_custom_type_kw(token)) {
    return TYPE_KW;
  }

  if (is_branch_keyword(token)) {
    return BRANCH_KW;
  }

  if (is_terminating_keyword(token)) {
    return TERM_KW;
  }

  if (is_control_keyword(token)) {
    return CONTROL_KW;
  }

  if (is_program_block_kw(token)) {
    return PROGRAM_BLOCK_KW;
  }

  if (is_literal_keyword(token)) {
    return LITERAL_KW;
  }

  if (is_parenthetical_separator(token)) {
    return PAREN_SEP;
  }

  if (is_punctuational_separator(token)) {
    return PUNCT_SEP;
  }

  if (is_binary_math_operator(token)) {
    return BIN_MATH_OP;
  }

  if (is_unary_math_operator(token)) {
    return UNA_MATH_OP;
  }

  if (is_binary_assignment_operator(token)) {
    return BIN_ASSIGN_OP;
  }

  if (is_unary_assignment_operator(token)) {
    return UNA_ASSIGN_OP;
  }

  if (is_comparison_operator(token)) {
    return COMP_OP;
  }

  if (is_binary_logical_operator(token)) {
    return BIN_LOG_OP;
  }

  if (is_unary_logical_operator(token)) {
    return UNA_LOG_OP;
  }

  return UNCLASSIFIABLE;
}

static MiniTokenName name_identifier(char *token) {
  char first = token[0];
  switch (first) {
    case 'M':
      return MINI_EXT_ID;
    case 'C':
      return C_ID; 
    default:
      if (isupper(first)) {
        return MINI_CONST_ID;
      } else {
        return MINI_ID;
      }
  }
}

static MiniTokenName name_type(char *token) {
  // TODO: Implement a type namer for arbitratry types. Perhaps at semantic analysis stage?
  char first = token[0];
  char second = token[1];
  switch (first) {
    case '<':
      switch (second) {
        case '>':
          return VOID;
        case '#':
          return INT;
        case '%':
          return FLOAT;
        case '"':
          return STR;
        case 'B':
          return BOOL;
        case 'S':
          return STREAM;
        default:
          return CUSTOM_T;
      }
    case '[':
      size_t len = strlen(token);
      for (size_t i = 0; i < len; i++) {
        if (token[i] == ':') {
          return DICT_T;
        }
      }
      return LIST_T;
   case '{':
      switch (second) {
        case 'E':
          if (token[2] == '}') {
            return ENUM_T;
          } else {
            return STRUCT_T;
          }
        case 'U':
          if (token[2] == '}') {
            return UNION_T;
          } else {
            return STRUCT_T;
          }
        default:
          return STRUCT_T; 
      }
   default:
      return VOID; // Is void a good default?
  }
}

static MiniTokenName name_branch(char *token) {
  char first = token[0];
  switch (first) {
    case '?':
      return IF;
    case '#':
      return SWITCH;
    case '@':
      return LOOP;
  }
  return TOKEN_UNDETERMINED;
}

static MiniTokenName name_terminator(char *token) {
  char type = token[1];
  switch (type) {
    case '{':
      return END_MODULE;
    case '<':
      return END_MAIN;
    case '.':
      return ELSE;
    case '?':
      if (token[0] == '|') {
        return ELSE_IF;
      } else {
        return END_IF;
      }
    case '=':
      return CASE;
    case '#':
      return END_SWITCH;
    case '@':
      return END_LOOP;
    case '$':
      return END_FUNC;
  }
  return TOKEN_UNDETERMINED;
}

static MiniTokenName name_control(char *token) {
  char first = token[0];
  size_t len = strlen(token);
  switch (first) {
    case 'M':
      return M_IMPORT;
    case 'C':
      return C_IMPORT;
    case ':':
      return IMPORT;
    case '!':
      return READ_WRITE;
    case '-':
      return REDIRECT;
    case '$':
      return CALL;
    case '<':
      return RETURN;
    case '.':
      if (len == 1) {
        return BREAK;
      } else {
        return CONTINUE;
      }
  }
  return TOKEN_UNDETERMINED;
}

static MiniTokenName name_program_block(char *token) {
  char first = token[0];
  switch (first) {
    case '}':
      return MODULE;
    case '>':
      return MAIN;
    case '$':
      return FUNC;
    case '!':
      return MAIN_DECLARATION;
  }
  return TOKEN_UNDETERMINED;
}

static MiniTokenName name_kw_literal(char *token) {
  char first = token[0];
  switch (first) {
    case 'T':
      return TRUE;
    case 'F':
      return FALSE;
    case 'N':
      return NUL;
    case '.':
      return STDIO;
    case '[':
      return ARGV;
    case '_':
      return DEFAULT;
  }
  return TOKEN_UNDETERMINED;
}

static MiniTokenName name_paren_sep(char *token) {
  char first = token[0];
  switch (first) {
    case '(':
      return LEFT_PAREN;
    case ')':
      return RIGHT_PAREN;
    case '[':
      return LEFT_BRACKET;
    case ']':
      return RIGHT_BRACKET;
    case '{':
      return LEFT_BRACE;
    case '}':
      return RIGHT_BRACE;
    case '|':
      return VERTICAL_BAR;
  }
  return TOKEN_UNDETERMINED;
}

static MiniTokenName name_punct_sep(char *token) {
  char first = token[0];
  switch (first) {
    case ',':
      return COMMA;
    case ';':
      return SEMICOLON;
    case ':':
      return COLON;
  }
  return TOKEN_UNDETERMINED;
}

static MiniTokenName name_bin_math_op(char *token) {
  char first = token[0];
  size_t len = strlen(token);
  switch (first) {
    case '+':
      return PLUS;
    case '-':
      return MINUS;
    case '*':
      if (len == 1) {
        return TIMES;
      } else {
        return POW;
      }
    case '/':
      return DIV;
    case '%':
      return MOD;
  } 
  return TOKEN_UNDETERMINED;
}

static MiniTokenName name_una_math_op(char *token) {
  char first = token[0];
  switch (first) {
    case '\\':
      return SQRT;
    case '^':
      return DEREFERENCE;
    case '@':
      return ADDRESS; 
  }
  return TOKEN_UNDETERMINED;
}

static MiniTokenName name_bin_assign_op(char *token) {
  char first = token[0];
  switch (first) {
    case ':':
      return ASSIGN;
    case '+':
      return PLUS_ASSIGN;
    case '-':
      return MINUS_ASSIGN;
    case '*':
      return TIMES_ASSIGN;
    case '/':
      return DIV_ASSIGN;
    case '%':
      return MOD_ASSIGN;
  }
  return TOKEN_UNDETERMINED;
}

static MiniTokenName name_una_assign_op(char *token) {
  char first = token[0];
  switch (first) {
    case '+':
      return INCREMENT;
    case '-':
      return DECREMENT;
  }
  return TOKEN_UNDETERMINED;
}

static MiniTokenName name_comp_op(char *token) {
  char first = token[0];
  size_t len = strlen(token);
  switch (first) {
    case '=':
      return EQUALS;
    case '~':
      return NOT_EQUAL;
    case '<':
      if (len == 1) {
        return LESS_THAN;
      } else {
        return LESS_EQUAL;
      }
    case '>':
      if (len == 1) {
        return GREATER_THAN;
      } else {
        return GREATER_EQUAL;
      }
  }
  return TOKEN_UNDETERMINED;
}

static MiniTokenName name_bin_log_op(char *token) {
  char first = token[0];
  switch (first) {
    case '&':
      return AND;
    case 'V':
      return OR;
  }
  return TOKEN_UNDETERMINED;
}

static MiniTokenName name_una_log_op(char *token) {
  return NOT;
}

static MiniTokenName name_literal(char *token) {
  if (token[0] == '"') {
    return STRING_LITERAL;
  }
  size_t len = strlen(token);
  for (size_t i = 0; i < len; i++) {
    if (token[i] == '.') {
      return FLOAT_LITERAL;
    }
  }
  return INT_LITERAL;
}

static MiniTokenName name_irrelevant(char *token) {
  return IRRELEVANT;
}


static MiniTokenName name_token(char *token, MiniTokenCat category) {
  switch (category) {
    case IDENTIFIER:
      return name_identifier(token);
    case TYPE_KW:
      return name_type(token);
    case BRANCH_KW:
      return name_branch(token);
    case TERM_KW:
      return name_terminator(token);
    case CONTROL_KW:
      return name_control(token);
    case PROGRAM_BLOCK_KW:
      return name_program_block(token);
    case LITERAL_KW:
      return name_kw_literal(token); 
    case PAREN_SEP:
      return name_paren_sep(token);
    case PUNCT_SEP:
      return name_punct_sep(token);
    case BIN_MATH_OP:
      return name_bin_math_op(token);
    case UNA_MATH_OP:
      return name_una_math_op(token);
    case BIN_ASSIGN_OP:
      return name_bin_assign_op(token);
    case UNA_ASSIGN_OP:
      return name_una_assign_op(token);
    case COMP_OP:
      return name_comp_op(token);
    case BIN_LOG_OP:
      return name_bin_log_op(token);
    case UNA_LOG_OP:
      return name_una_log_op(token);
    case LITERAL:
      return name_literal(token);
    default:
      return name_irrelevant(token);
  } 
}



MiniStatus tokenize(char *input_file, char *output_file, MiniHeadToken *head_token, int verbose) {
  if (verbose) {
    printf("Beginning tokenization\n");
  }
  size_t filename_len = strlen(input_file);
  if (output_file[0] == '\0') {
    strcpy(output_file, input_file);
    output_file[filename_len - 4] = 't';
    output_file[filename_len - 3] = 'o';
    output_file[filename_len - 2] = 'k';
    output_file[filename_len - 1] = 'e';
  }
  if (verbose) {
    printf("Output file: %s\n", output_file);
  }
  FILE *output_ptr;
  output_ptr = fopen(output_file, "w");
  fprintf(output_ptr, "Line:Col Token Category Name\n");
  FILE *input_ptr;
  char line_buffer[MAX_LINE_LENGTH + 2];

  input_ptr = fopen(input_file, "r");
  if (input_ptr == NULL) {
    printf("File Error: Preprocessed file %s couldn't be found!\n", input_file);
    fclose(input_ptr);
    fclose(output_ptr);
    return FILE_NOT_FOUND;
  }


  int line_count = 0;
  size_t line_length;
  int category;
  int name;
  while (fgets(line_buffer, MAX_LINE_LENGTH + 2, input_ptr) != NULL) {
    line_length = strcspn(line_buffer, "\n");
    line_buffer[line_length] = '\0';
    char substring_buffer[line_length + 1]; // substring_buffer needs to be able to hold line_length printable characters
                                            // and thus one more slot is required for the null terminator
    size_t starting_index = 0;
    for (size_t i = 0; i < line_length; i++) {
      if (starting_index >= line_length) {
        break;
      }
      size_t copy_amount = line_length - i - starting_index;
      char *copy = line_buffer + starting_index;
      strncpy(substring_buffer, copy, copy_amount);
      substring_buffer[copy_amount] = '\0';
      //printf("DEBUG: Current substring: %s\n", substring_buffer);
      category = categorize_token(substring_buffer);
      if (category == UNCLASSIFIABLE) {
        if (copy_amount <= 1) {
          printf("Lexical error: Unclassifiable token beginning with %s approximately on line %d\n", substring_buffer, line_count + 1);
          fclose(input_ptr);
          fclose(output_ptr);
          return INVALID_SYNTAX;
        }
      } else {
        name = name_token(substring_buffer, category);
        printf("DEBUG: Token: %s, Category: %d, Name: %d\n", substring_buffer, category, name);
        //printf("Category: %d\n", category);
        fprintf(output_ptr, "%d:%lu %s %d %d\n", line_count + 1, starting_index, substring_buffer, category, name); 
        if (category != COMMENT && category != WHITESPACE) {
          MiniStatus status;
          MiniToken *new_token = alloc_token(&status);
          if (status != SUCCESS) return status;
          status = init_token(new_token, substring_buffer, category, name);
          if (status != SUCCESS) return status;
          add_token(head_token, new_token);
        }
        starting_index += line_length - i - starting_index;
        i = -1;
      }
    }
    line_count++;
  }

  if (line_count == 0) {
    printf("File Error: Preprocessed file %s was empty!\n", input_file);
    fclose(input_ptr);
    fclose(output_ptr);
    return FILE_EMPTY;
  }
  fclose(input_ptr);
  fclose(output_ptr);
  if (verbose) {
    printf("Tokenization complete\n");
  }
  return SUCCESS;
}
  

