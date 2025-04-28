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

#ifndef MINIMAL_SYNTAX_H
#define MINIMAL_SYNTAX_H

#include "tokens.h"

/* Minimal syntax construct numbering scheme: The number of digits denotes
 * how many layers deep one needs to go to find the concept. (Exception: SOURCE = 0),
 * which is not one layer deep but zero layers deep. The first branch of a concept with
 * number X is numbered X1, the second branch X2 and so on. This way the branches are numbered
 * with unique numbers and each successive digit specified which branch was chosen at each
 * juncture. If a branch has no more children, the number of the branch followed by an 'x' is
 * still listed where the children would be in case that branch is ever needed. After that the
 * branch number is omitted to avoid clutter (Meaning that there will never be a number with two
 * x's after it)
 */

// XXX: Determine whether EXISTENCE_DECLARATION and INITIALIZATION are needed distinctions

typedef enum minimal_nonterminal_constructs {
  NON_TERM_UNDETERMINED = -1,
  SOURCE = 0, // Collection of source code files
  MAIN_FILE = 1, // Contains an optional module part and the main program
  MODULE_FILE = 2, // Only contains a module part
  MAIN_PART = 11, // The entry point of the executable
  MODULE_PART = 12, // A series of imports, declarations and definitions
  SEQUENCE = 111,
  MODULE_SEQUENCE = 121,
  STATEMENT = 1111, // Expresses an action
  BRANCH = 1112, // A sequence inside a conditional block
  IMPORTING = 1211,
  TYPE_ALIASING = 1212,
  SUBPROGRAM = 1213,
  MODULE_DECLARATION = 1214, // Declares the existence of an object and potentially assigns it a value;
  DECLARATION = 11111,
  DESIGNATION = 11112, // Designates a value to an object
  CONTROL = 11113, // Controls execution and reading/writing (continue, return, printf etc.)
  //DEFINITION = 11114, // Defines the meaning of some symbol // XXX: Deprecated??
  IF_BLOCK = 11121,
  SWITCH_BLOCK = 11122,
  LOOP_BLOCK = 11123,
  FILE_IMPORT = 12111,
  M_FILE_IMPORT = 12112,
  C_FILE_IMPORT = 12113,
  TYPE_EXPR = 12121,
  PARAM_LIST = 12131,
  COLLECTION = 12141,
  PRIMARY_EXPRESSION = 111111, // Evaluates to a value, simple or complex
  ASSIGNMENT = 111121,
  INCREMENTATION = 111122,
  IN_OUT_CTRL = 111131, // Reading from stdin/file and writing to stdout/file
  FLOW_CTRL = 111132, // Breaking, Continuing, Returning
  FUNC_CALL = 111133, // Minimal function call
  ELIF_BLOCK = 111211,
  ELSE_BLOCK = 111212,
  CASE_BLOCK = 111221,
  FOR_LOOP = 111231,
  WHILE_LOOP = 111232,
  EXPRESSION = 1111111, // Evaluates to a value, complex
  INDEXING = 1111112,
  SIZEOF = 1111113,
  ARGUMENT_LIST = 1111331,
  //FIELD_LIST = 1111431,
  LIST = 121411,
  DICT = 121412,
  //STRUCTURE = 121413,
  ARITHMETIC_EXPR = 11111111, // A combination of mathematical operations
  //STRING_EXPR = 11111112, // A combination of string operations
  LOGICAL_EXPR = 11111113, // A combination of relational expression and logical operators
  ARITH_OPERAND = 111111111,
  STRING_OPERAND = 111111112,
  LOGICAL_OPERAND = 111111113
} MiniNonTerm;


void print_construct_category(int construct, int type);
void file_print_construct_category(FILE *file_ptr, int construct, int type);

//extern const int VARIABLE_LIST_INCREMENT;
extern const int TREE_INDENT_WIDTH;

/*
typedef struct minimal_variable {
  int type;
  char *identifier;
} MiniVar;

int init_variable(MiniVar *var, int type, char *identifier);
void free_variable(MiniVar *var);

typedef struct minimal_variable_list {
  MiniVar *variables;
  uint64_t capacity;
  uint64_t currently_occupied; 
} MiniVarList;

int init_var_list(MiniVarList *list);
void free_var_list(MiniVarList *list);
int add_var(MiniVarList *list, MiniVar var);
*/

typedef union minimal_grammatical_construct {
  MiniToken token;
  MiniNonTerm non_terminal;
} MiniGramCons;

typedef enum minimal_grammatical_construct_types {
  INACTIVE = 0,
  TOKEN = 1,
  NON_TERMINAL = 2
} MiniConsType;

typedef struct minimal_syntax_tree {
  MiniGramCons data;
  MiniConsType data_type;
  struct minimal_syntax_tree *child;
  struct minimal_syntax_tree *sibling;
} MiniSyntaxTree;

typedef enum minimal_node_relation {
  SIBLING = 0,
  CHILD
} MiniRelation;

//extern uint64_t g_token_index;
//extern uint64_t g_max_token_index;
//extern MiniToken *g_current_token;

//void get_current_token(MiniTokenList token_list, MiniToken *token);
//void increment_token_index();
//int find_next_relevant_token(MiniTokenList token_list);

MiniGramCons *alloc_gram_construct(MiniStatus *status);
MiniSyntaxTree *alloc_syntax_tree(MiniStatus *status);
MiniStatus init_syntax_tree(MiniSyntaxTree *tree, MiniGramCons *constr, MiniConsType type);
void free_syntax_tree(MiniSyntaxTree *tree);
void add_node(MiniSyntaxTree *target_node, MiniSyntaxTree *new_node, MiniRelation relation);
void print_syntax_tree(MiniSyntaxTree *tree, int indent_multiplier);
void file_print_syntax_tree(FILE *file_ptr, MiniSyntaxTree *tree, int indent_multilplier);

MiniStatus generate_ast(char *input_file, char *output_file, MiniHeadToken *head_token, MiniSyntaxTree *root, int verbose);

#endif
