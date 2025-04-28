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
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "inc/retcodes.h"
#include "inc/tokens.h"
#include "inc/syntax.h"

//const int VARIABLE_LIST_INCREMENT = 10;
const int TREE_INDENT_WIDTH = 2;

void print_construct_category(int construct, int type) {
  if (type == TOKEN) {
    switch (construct) {
      case IDENTIFIER: printf("Identifier"); return;
      case TYPE_KW: printf("Type Keyword"); return;
      case BRANCH_KW: printf("Branch Keyword"); return;
      case TERM_KW: printf("Terminating Keyword"); return;
      case CONTROL_KW: printf("Control Keyword"); return;
      case PROGRAM_BLOCK_KW: printf("Program Block Keyword"); return;
      case LITERAL_KW: printf("Literal Keyword"); return;
      case PAREN_SEP: printf("Parenthetical Separator"); return;
      case PUNCT_SEP: printf("Punctuational Separator"); return;
      case BIN_MATH_OP: printf("Binary Math Operator"); return;
      case UNA_MATH_OP: printf("Unary Math Operator"); return;
      case BIN_ASSIGN_OP: printf("Binary Assignment Operator"); return;
      case UNA_ASSIGN_OP: printf("Unary Assignment Operator"); return;
      case COMP_OP: printf("Comparison Operator"); return;
      case BIN_LOG_OP: printf("Binary Logical Operator"); return;
      case UNA_LOG_OP: printf("Unary Logical Operator"); return;
      case LITERAL: printf("Literal"); return;
      default: printf("Unclassifiable"); return;
    }
  } else if (type == NON_TERMINAL) {
    switch (construct) {
      case SOURCE: printf("Source"); return;
      case MAIN_FILE: printf("Main File"); return;
      case MODULE_FILE: printf("Module File"); return;
      case MAIN_PART: printf("Main Part"); return;
      case MODULE_PART: printf("Module Part"); return;
      case SEQUENCE: printf("Sequence"); return;
      case MODULE_SEQUENCE: printf("Module Sequence"); return;
      case STATEMENT: printf("Statement"); return;
      case BRANCH: printf("Branch"); return;
      case IMPORTING: printf("Import"); return;
      case TYPE_ALIASING: printf("Type Alias"); return;
      case SUBPROGRAM: printf("Subprogram"); return;
      case MODULE_DECLARATION: printf("Module Declaration"); return;
      case DECLARATION: printf("Declaration"); return;
      case DESIGNATION: printf("Designation"); return;
      case CONTROL: printf("Control"); return;
      case IF_BLOCK: printf("If-Block"); return;
      case SWITCH_BLOCK: printf("Switch-Block"); return;
      case LOOP_BLOCK: printf("Loop-Block"); return;
      case FILE_IMPORT: printf("Minimal Module Import"); return;
      case M_FILE_IMPORT: printf("Minimal Stdlib Import"); return;
      case C_FILE_IMPORT: printf("C Import"); return;
      case TYPE_EXPR: printf("Type Expression"); return;
      case PARAM_LIST: printf("Parameter List"); return;
      case COLLECTION: printf("Collection"); return;
      case PRIMARY_EXPRESSION: printf("Primary Expression"); return;
      case ASSIGNMENT: printf("Assignment"); return;
      case INCREMENTATION: printf("Incrementation"); return;
      case IN_OUT_CTRL: printf("Input/Output -Control"); return;
      case FLOW_CTRL: printf("Flow Control"); return;
      case FUNC_CALL: printf("Function Call"); return;
      case ELIF_BLOCK: printf("Else If -Block"); return;
      case ELSE_BLOCK: printf("Else-Block"); return;
      case CASE_BLOCK: printf("Case-Block"); return;
      case FOR_LOOP: printf("For-Loop"); return;
      case WHILE_LOOP: printf("While-Loop"); return;
      case EXPRESSION: printf("Expression"); return;
      case INDEXING: printf("Indexing"); return;
      case SIZEOF: printf("Sizeof"); return;
      case ARGUMENT_LIST: printf("Argument List"); return;
      case LIST: printf("List"); return;
      case DICT: printf("Associative Array"); return;
      case ARITHMETIC_EXPR: printf("Arithmetic Expression"); return;
      case STRING_EXPR: printf("String Expression"); return;
      case LOGICAL_EXPR: printf("Logical Expression"); return;
      case ARITH_OPERAND: printf("Arithmetic Operand"); return;
      case STRING_OPERAND: printf("String Operand"); return;
      case LOGICAL_OPERAND: printf("Logical Operand"); return;
      default: printf("Unclassifiable"); return;
    }

  } else {
    printf("Invalid type!\n");
  }
  return;
}

void file_print_construct_category(FILE *file_ptr, int construct, int type) {
  if (type == TOKEN) {
    switch (construct) {
      case IDENTIFIER: fprintf(file_ptr, "Identifier"); return;
      case TYPE_KW: fprintf(file_ptr, "Type Keyword"); return;
      case BRANCH_KW: fprintf(file_ptr, "Branch Keyword"); return;
      case TERM_KW: fprintf(file_ptr, "Terminating Keyword"); return;
      case CONTROL_KW: fprintf(file_ptr, "Control Keyword"); return;
      case PROGRAM_BLOCK_KW: fprintf(file_ptr, "Program Block Keyword"); return;
      case LITERAL_KW: fprintf(file_ptr, "Literal Keyword"); return;
      case PAREN_SEP: fprintf(file_ptr, "Parenthetical Separator"); return;
      case PUNCT_SEP: fprintf(file_ptr, "Punctuational Separator"); return;
      case BIN_MATH_OP: fprintf(file_ptr, "Binary Math Operator"); return;
      case UNA_MATH_OP: fprintf(file_ptr, "Unary Math Operator"); return;
      case BIN_ASSIGN_OP: fprintf(file_ptr, "Binary Assignment Operator"); return;
      case UNA_ASSIGN_OP: fprintf(file_ptr, "Unary Assignment Operator"); return;
      case COMP_OP: fprintf(file_ptr, "Comparison Operator"); return;
      case BIN_LOG_OP: fprintf(file_ptr, "Binary Logical Operator"); return;
      case UNA_LOG_OP: fprintf(file_ptr, "Unary Logical Operator"); return;
      case LITERAL: fprintf(file_ptr, "Literal"); return;
      default: fprintf(file_ptr, "Unclassifiable"); return;
    }
  } else if (type == NON_TERMINAL) {
    switch (construct) {
      case SOURCE: fprintf(file_ptr, "Source"); return;
      case MAIN_FILE: fprintf(file_ptr, "Main File"); return;
      case MODULE_FILE: fprintf(file_ptr, "Module File"); return;
      case MAIN_PART: fprintf(file_ptr, "Main Part"); return;
      case MODULE_PART: fprintf(file_ptr, "Module Part"); return;
      case SEQUENCE: fprintf(file_ptr, "Sequence"); return;
      case MODULE_SEQUENCE: fprintf(file_ptr, "Module Sequence"); return;
      case STATEMENT: fprintf(file_ptr, "Statement"); return;
      case BRANCH: fprintf(file_ptr, "Branch"); return;
      case IMPORTING: fprintf(file_ptr, "Import"); return;
      case TYPE_ALIASING: fprintf(file_ptr, "Type Alias"); return;
      case SUBPROGRAM: fprintf(file_ptr, "Subprogram"); return;
      case MODULE_DECLARATION: fprintf(file_ptr, "Module Declaration"); return;
      case DECLARATION: fprintf(file_ptr, "Declaration"); return;
      case DESIGNATION: fprintf(file_ptr, "Designation"); return;
      case CONTROL: fprintf(file_ptr, "Control"); return;
      case IF_BLOCK: fprintf(file_ptr, "If-Block"); return;
      case SWITCH_BLOCK: fprintf(file_ptr, "Switch-Block"); return;
      case LOOP_BLOCK: fprintf(file_ptr, "Loop-Block"); return;
      case FILE_IMPORT: fprintf(file_ptr, "Minimal Module Import"); return;
      case M_FILE_IMPORT: fprintf(file_ptr, "Minimal Stdlib Import"); return;
      case C_FILE_IMPORT: fprintf(file_ptr, "C Import"); return;
      case TYPE_EXPR: fprintf(file_ptr, "Type Expression"); return;
      case PARAM_LIST: fprintf(file_ptr, "Parameter List"); return;
      case COLLECTION: fprintf(file_ptr, "Collection"); return;
      case PRIMARY_EXPRESSION: fprintf(file_ptr, "Primary Expression"); return;
      case ASSIGNMENT: fprintf(file_ptr, "Assignment"); return;
      case INCREMENTATION: fprintf(file_ptr, "Incrementation"); return;
      case IN_OUT_CTRL: fprintf(file_ptr, "Input/Output -Control"); return;
      case FLOW_CTRL: fprintf(file_ptr, "Flow Control"); return;
      case FUNC_CALL: fprintf(file_ptr, "Function Call"); return;
      case ELIF_BLOCK: fprintf(file_ptr, "Else If -Block"); return;
      case ELSE_BLOCK: fprintf(file_ptr, "Else-Block"); return;
      case CASE_BLOCK: fprintf(file_ptr, "Case-Block"); return;
      case FOR_LOOP: fprintf(file_ptr, "For-Loop"); return;
      case WHILE_LOOP: fprintf(file_ptr, "While-Loop"); return;
      case EXPRESSION: fprintf(file_ptr, "Expression"); return;
      case INDEXING: fprintf(file_ptr, "Indexing"); return;
      case SIZEOF: fprintf(file_ptr, "Sizeof"); return;
      case ARGUMENT_LIST: fprintf(file_ptr, "Argument List"); return;
      case LIST: fprintf(file_ptr, "List"); return;
      case DICT: fprintf(file_ptr, "Associative Array"); return;
      case ARITHMETIC_EXPR: fprintf(file_ptr, "Arithmetic Expression"); return;
      case STRING_EXPR: fprintf(file_ptr, "String Expression"); return;
      case LOGICAL_EXPR: fprintf(file_ptr, "Logical Expression"); return;
      case ARITH_OPERAND: fprintf(file_ptr, "Arithmetic Operand"); return;
      case STRING_OPERAND: fprintf(file_ptr, "String Operand"); return;
      case LOGICAL_OPERAND: fprintf(file_ptr, "Logical Operand"); return;
      default: fprintf(file_ptr, "Unclassifiable"); return;
    }

  } else {
    fprintf(file_ptr, "Invalid type!\n");
  }
  return;
}

/*
int init_variable(MiniVar *var, int var_type, char *name) {
  size_t length = strlen(name) + 1; // + 1 to account for null terminator
  var->identifier = malloc(length * sizeof(char));
  if (var->identifier == NULL) {
    return ALLOCATION_FAIL;
  }
  strcpy(var->identifier, name);
  var->type = var_type;
  return SUCCESS;
}

void free_variable(MiniVar *var) {
  free(var->identifier);
  return;
}

int init_var_list(MiniVarList *varlist) {
  varlist->variables = malloc(VARIABLE_LIST_INCREMENT * sizeof(MiniVar));
  if (varlist->variables == NULL) {
    return ALLOCATION_FAIL;
  }
  varlist->capacity = VARIABLE_LIST_INCREMENT;
  varlist->currently_occupied = 0;
  return SUCCESS;
}

int add_var(MiniVarList *varlist, MiniVar var) {
  if (varlist->currently_occupied == varlist->capacity) {
    varlist->capacity += VARIABLE_LIST_INCREMENT;
    varlist->variables = realloc(varlist->variables, varlist->capacity * sizeof(MiniVar));
    if (varlist->variables == NULL) {
      return REALLOCATION_FAIL;
    }
  }

  varlist->variables[varlist->currently_occupied] = var;
  varlist->currently_occupied++;
  return SUCCESS;
}

void free_var_list(MiniVarList *varlist) {
  for (int i = 0; i < varlist->currently_occupied; i++) {
    free_variable(&(varlist->variables[i]));
  }
  free(varlist->variables);
  return;
}

void print_var_list(MiniVarList *varlist) {
  uint64_t items = varlist->currently_occupied;
  for (int i = 0; i < items; i++) {
    MiniVar current = varlist->variables[i];
    printf("Variable %d: %s ", i, current.identifier);
    printf(" Type: %d\n", current.type);
  }
  return;
}
*/

MiniGramCons *alloc_gram_construct(MiniStatus *status) {
  MiniGramCons *construct = malloc(sizeof(MiniGramCons));
  memset(construct, 0, sizeof(MiniGramCons));
  if (construct == NULL) {
    printf("alloc_gram_construct: Memory Error: Failed to allocate spce for new grammatical construct\n");
    *status = ALLOCATION_FAIL;
    return NULL;
  }
  *status = SUCCESS;
  return construct;
}

MiniSyntaxTree *alloc_syntax_tree(MiniStatus *status) {
  MiniSyntaxTree *tree = malloc(sizeof(MiniSyntaxTree));
  if (tree == NULL) {
    printf("alloc_syntax_tree: Memory Error: Failed to allocate space for new syntax tree node\n");
    *status = ALLOCATION_FAIL;
    return NULL;
  }
  *status = SUCCESS;
  return tree;
}

// A tree node is initialized with COPIED information
// from a token, so that the tokens can be freed
// without affecting the tree after the tree has been
// generated. 
MiniStatus init_syntax_tree(MiniSyntaxTree *tree, MiniGramCons *constr, MiniConsType type) {
  if (type == TOKEN) {
    //construct_type = MINITOK;
    //MiniToken *tree_token = &(tree->data.token);
    //tree_token = malloc(sizeof(MiniToken));
    //memcpy(tree_token, g_current_token, sizeof(MiniToken));
    tree->data.token = constr->token;
  } else { 
    //construct_type = SYNTAX_CONSTRUCT;
    //MiniNonTerm *tree_nonterm = &(tree->data.construct);
    //tree_nonterm = malloc(sizeof(MiniNonTerm));
    //memcpy(tree_nonterm, &gram_const, sizeof(MiniNonTerm));
    tree->data.non_terminal = constr->non_terminal;
  }
  free(constr);
  //MiniConsType *tree_type = &(tree->data_type);
  //tree_type = malloc(sizeof(MiniConsType));
  //memcpy(tree_type, &construct_type, sizeof(MiniConsType));
  //tree->data_type = construct_type;
  tree->data_type = type;
  tree->child = NULL;
  tree->sibling = NULL;
  return SUCCESS;
}

void add_node(MiniSyntaxTree *target_node, MiniSyntaxTree *new_node, MiniRelation relation) {
  if (relation == CHILD) {
    target_node->child = new_node;
  } else if (relation == SIBLING){
    target_node->sibling = new_node;
  } else {
    return;
  }
  return;
}

void free_syntax_tree(MiniSyntaxTree *tree) {
  if (tree == NULL) {
    return;
  }
  free_syntax_tree(tree->child);
  free_syntax_tree(tree->sibling);
  free(tree);
  return;
}

void print_syntax_tree(MiniSyntaxTree *tree, int indent_multiplier) {
  if (tree == NULL) {
    return;
  }
  for (int i = 0; i < indent_multiplier * TREE_INDENT_WIDTH; i++) {
    putchar(' ');
  }
  if (tree->data_type == TOKEN) {
    printf("["); 
    print_construct_category(tree->data.token.category, tree->data_type);
    printf(": %s]\n", tree->data.token.string_repr);
  } else {
    printf("[");
    print_construct_category(tree->data.non_terminal, tree->data_type);
    printf("]\n");
  }
  print_syntax_tree(tree->child, indent_multiplier + 1);
  print_syntax_tree(tree->sibling, indent_multiplier);
  return;
}

void file_print_syntax_tree(FILE *file_ptr, MiniSyntaxTree *tree, int indent_multiplier) {
  if (tree == NULL) {
    return;
  }
  for (int i = 0; i < indent_multiplier * TREE_INDENT_WIDTH; i++) {
    fprintf(file_ptr, " ");
  }
  if (tree->data_type == TOKEN) {
    fprintf(file_ptr, "["); 
    file_print_construct_category(file_ptr, tree->data.token.category, tree->data_type);
    fprintf(file_ptr, ": %s]\n", tree->data.token.string_repr);
  } else {
    fprintf(file_ptr, "[");
    file_print_construct_category(file_ptr, tree->data.non_terminal, tree->data_type);
    fprintf(file_ptr, "]\n");
  }
  file_print_syntax_tree(file_ptr, tree->child, indent_multiplier + 1);
  file_print_syntax_tree(file_ptr, tree->sibling, indent_multiplier);
  return;
}
