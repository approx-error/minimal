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
#include <stdbool.h>
#include <stdlib.h>

#include "inc/retcodes.h"
#include "inc/tokens.h"
#include "inc/syntax.h"
#include "inc/parser-utils.h"

static MiniStatus source(MiniSyntaxTree *, MiniToken *);
static MiniStatus main_file(MiniSyntaxTree *, MiniToken *);
static MiniStatus module_file(MiniSyntaxTree *, MiniToken *, MiniToken **);
static MiniStatus module_part(MiniSyntaxTree *, MiniToken *, MiniToken **);
static MiniStatus module_sequence(MiniSyntaxTree *, MiniToken *, MiniToken **);
static MiniStatus import(MiniSyntaxTree *, MiniToken *, MiniToken **);
static MiniStatus type_aliasing(MiniSyntaxTree *, MiniToken *, MiniToken **);
static MiniStatus module_declaration(MiniSyntaxTree *, MiniToken *, MiniToken **);
static MiniStatus subprogram(MiniSyntaxTree *, MiniToken *, MiniToken **);
static MiniStatus parameter_list(MiniSyntaxTree *, MiniToken *, MiniToken **);
static MiniStatus type(MiniSyntaxTree *, MiniToken *);
static MiniStatus collection(MiniSyntaxTree *, MiniToken *, MiniToken **);
static MiniStatus list(MiniSyntaxTree *, MiniToken *, MiniToken **);
static MiniStatus dictionary(MiniSyntaxTree *, MiniToken *, MiniToken **);
static MiniStatus primary_expression(MiniSyntaxTree *, MiniToken *, MiniToken **);
static MiniStatus expression(MiniSyntaxTree *, MiniToken *, MiniToken **);
static MiniStatus arithmetic_expression(MiniSyntaxTree *, MiniToken *, MiniToken **);
static MiniStatus logical_expression(MiniSyntaxTree *, MiniToken *, MiniToken **);
static MiniStatus main_part(MiniSyntaxTree *, MiniToken *);
static MiniStatus sequence(MiniSyntaxTree *, MiniToken *, MiniToken **);
static MiniStatus statement(MiniSyntaxTree *, MiniToken *, MiniToken **);
static MiniStatus designation(MiniSyntaxTree *, MiniToken *, MiniToken **);
static MiniStatus assignment(MiniSyntaxTree *, MiniToken *, MiniToken **);
static MiniStatus incrementation(MiniSyntaxTree *, MiniToken *, MiniToken **);
static MiniStatus control(MiniSyntaxTree *, MiniToken *, MiniToken **);
static MiniStatus input_output_control(MiniSyntaxTree *, MiniToken *, MiniToken **);
static MiniStatus flow_control(MiniSyntaxTree *, MiniToken *, MiniToken **);
static MiniStatus function_call(MiniSyntaxTree *, MiniToken *, MiniToken **);
static MiniStatus argument_list(MiniSyntaxTree *, MiniToken *, MiniToken **);
static MiniStatus branch(MiniSyntaxTree *, MiniToken *, MiniToken **);
static MiniStatus if_block(MiniSyntaxTree *, MiniToken *, MiniToken **);
static MiniStatus elif_block(MiniSyntaxTree *, MiniToken *, MiniToken **);
static MiniStatus else_block(MiniSyntaxTree *, MiniToken *, MiniToken **);
static MiniStatus switch_block(MiniSyntaxTree *, MiniToken *, MiniToken **);
static MiniStatus case_block(MiniSyntaxTree *, MiniToken *, MiniToken **);
static MiniStatus loop_block(MiniSyntaxTree *, MiniToken *, MiniToken **);
static MiniStatus while_loop(MiniSyntaxTree *, MiniToken *, MiniToken **);
static MiniStatus for_loop(MiniSyntaxTree *, MiniToken *, MiniToken **);
static MiniStatus declaration(MiniSyntaxTree *, MiniToken *, MiniToken **);

// TODO: Remove many unneeded parse error print statements (they are unneeded) because
// we could only have gotten to that function if the parse didn't fail

// 1. Lexical analysis (are the symbols correct?)
// 2. Syntactic analysis (are the symbols correctly placed in relation to each other?)
// 3. Semantic analysis (does the thing the symbols are trying to convey make sense?)

// <declaration> ::= <type> (<mini-ID> | <mini-const-ID>) ("" | ":=" (<primary-expression> | <collection>))

static MiniStatus declaration(MiniSyntaxTree *current_node, MiniToken *current_token, MiniToken **token_carrier) {
  if (last_token(current_token)) return LAST_TOKEN;

  MiniStatus status;
  MiniSyntaxTree *cur_node = current_node;
  MiniSyntaxTree *new_node;
  MiniTokenCat category = TYPE_KW;
  MiniNonTerm corresp_nonterm = TYPE_EXPR;
  new_node = match_cat_and_add_nonterm_node(cur_node, current_token, &category, &corresp_nonterm, CHILD, NULL, &status);
  if (status == NONMATCHING_CATEGORY) {
    printf("Parse Error: Invalid declaration: Missing type keyword\n");
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;
  
  cur_node = new_node;
  status = type(cur_node, current_token);
  if (status != VALID_CONSTRUCT) return status;

  current_token = next_token(current_token, &status);
  if (status != SUCCESS) return status;

  MiniTokenName names[] = {MINI_ID, MINI_CONST_ID, -1};
  MiniTokenName name_match;
  new_node = match_and_add_term_node(cur_node, current_token, names, SIBLING, &name_match, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid declaration: Missing %s\n", desc_token(name_match));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;

  current_token = next_token(current_token, &status);
  if (status != SUCCESS) return status;

  MiniTokenName name = SEMICOLON;
  status = match_terminals(current_token, &name, NULL);
  if (status == NONMATCHING_TOKEN) {
    name = ASSIGN;
    new_node = match_and_add_term_node(cur_node, current_token, &name, SIBLING, NULL, &status);
    if (status == NONMATCHING_TOKEN) {
      printf("Parse Error: Invalid Declaration: Missing %s\n", desc_token(ASSIGN));
      return PARSE_ERROR;
    } else if (status != SUCCESS) return status;

    cur_node = new_node;

    current_token = next_token(current_token, &status);
    if (status != SUCCESS) return status;

    MiniToken *after_token2 = NULL;
    MiniTokenName names2[] = {LEFT_BRACKET, LEFT_BRACE, -1};
    MiniNonTerm corresp_nonterms[] = {COLLECTION, COLLECTION, -1};
    new_node = match_and_add_nonterm_node(cur_node, current_token, names2, corresp_nonterms, SIBLING, &name_match, &status);      
    if (status == NONMATCHING_TOKEN) {
      new_node = add_nonterm_node(cur_node, PRIMARY_EXPRESSION, SIBLING, &status);
      if (status != SUCCESS) return status;

      cur_node = new_node; 
     
      status = primary_expression(cur_node, current_token, &after_token2);
      if (status != VALID_CONSTRUCT) return status;

    } else if (status != SUCCESS) {
      return status;
    } else {

      cur_node = new_node;

      status = collection(cur_node, current_token, &after_token2);
      if (status != VALID_CONSTRUCT) return status;
    }

    *token_carrier = after_token2;
    return VALID_CONSTRUCT;

  } else if (status != SUCCESS) {
    return status;
  } else {
    cur_node = new_node;

    *token_carrier = current_token;
    return VALID_CONSTRUCT;
  }
  
  return VALID_CONSTRUCT;

}

// <for-loop> ::= "@@" <declaration> ";" <logical-expression> ; <incrementation> ":" <sequence> "~@"

static MiniStatus for_loop(MiniSyntaxTree *current_node, MiniToken *current_token, MiniToken **token_carrier) {
  if (last_token(current_token)) return LAST_TOKEN;

  MiniStatus status;
  MiniSyntaxTree *cur_node = current_node;
  MiniToken *cur_token = current_token;
  MiniSyntaxTree *new_node;
  MiniToken *after_token = NULL;
  MiniTokenName name = LOOP;
  new_node = match_and_add_term_node(cur_node, cur_token, &name, CHILD, NULL, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid for-loop: Missing %s\n", desc_token(LOOP));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;
  
  cur_node = new_node;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  new_node = add_nonterm_node(cur_node, DECLARATION, SIBLING, &status);
  if (status != SUCCESS) return status;

  cur_node = new_node;

  status = declaration(cur_node, cur_token, &after_token);
  if (status != VALID_CONSTRUCT) return status;

  cur_token = after_token;

  name = SEMICOLON;
  new_node = match_and_add_term_node(cur_node, cur_token, &name, SIBLING, NULL, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid for-loop: Missing %s\n", desc_token(SEMICOLON));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  new_node = add_nonterm_node(cur_node, LOGICAL_EXPR, SIBLING, &status);
  if (status != SUCCESS) return status;

  cur_node = new_node;

  status = logical_expression(cur_node, cur_token, &after_token);
  if (status != VALID_CONSTRUCT) return status;

  cur_token = after_token;

  new_node = match_and_add_term_node(cur_node, cur_token, &name, SIBLING, NULL, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid for-loop: Missing %s\n", desc_token(SEMICOLON));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  new_node = add_nonterm_node(cur_node, INCREMENTATION, SIBLING, &status);
  if (status != SUCCESS) return status;

  cur_node = new_node;

  status = incrementation(cur_node, cur_token, &after_token);
  if (status != VALID_CONSTRUCT) return status;

  cur_token = after_token;

  name = COLON;
  new_node = match_and_add_term_node(cur_node, cur_token, &name, SIBLING, NULL, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid for-loop: Missing %s\n", desc_token(COLON));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  new_node = add_nonterm_node(cur_node, SEQUENCE, SIBLING, &status);
  if (status != SUCCESS) return status;

  cur_node = new_node;

  status = sequence(cur_node, cur_token, &after_token);
  if (status != VALID_CONSTRUCT) return status;

  cur_token = after_token;

  name = END_LOOP;
  new_node = match_and_add_term_node(cur_node, cur_token, &name, SIBLING, NULL, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid for-loop: Missing %s\n", desc_token(END_LOOP));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  *token_carrier = cur_token;
  return VALID_CONSTRUCT;
}

// <while-loop> ::= "@@" <logical-expression> ":" <sequence> "~@"

static MiniStatus while_loop(MiniSyntaxTree *current_node, MiniToken *current_token, MiniToken **token_carrier) {
  if (last_token(current_token)) return LAST_TOKEN;

  MiniStatus status;
  MiniSyntaxTree *cur_node = current_node;
  MiniToken *cur_token = current_token;
  MiniSyntaxTree *new_node;
  MiniToken *after_token = NULL;
  MiniTokenName name = LOOP;
  new_node = match_and_add_term_node(cur_node, cur_token, &name, CHILD, NULL, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid while-loop: Missing %s\n", desc_token(LOOP));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;
  
  cur_node = new_node;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  new_node = add_nonterm_node(cur_node, LOGICAL_EXPR, SIBLING, &status);
  if (status != SUCCESS) return status;

  cur_node = new_node;

  status = logical_expression(cur_node, cur_token, &after_token);
  if (status != VALID_CONSTRUCT) return status;

  cur_token = after_token;

  name = COLON;
  new_node = match_and_add_term_node(cur_node, cur_token, &name, SIBLING, NULL, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid while-loop: Missing %s\n", desc_token(COLON));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  new_node = add_nonterm_node(cur_node, SEQUENCE, SIBLING, &status);
  if (status != SUCCESS) return status;

  cur_node = new_node;

  status = sequence(cur_node, cur_token, &after_token);
  if (status != VALID_CONSTRUCT) return status;

  cur_token = after_token;

  name = END_LOOP;
  new_node = match_and_add_term_node(cur_node, cur_token, &name, SIBLING, NULL, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid while-loop: Missing %s\n", desc_token(END_LOOP));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  *token_carrier = cur_token;
  return VALID_CONSTRUCT;
}



// <loop-block> ::= <while-loop> | <for-loop>

static MiniStatus loop_block(MiniSyntaxTree *current_node, MiniToken *current_token, MiniToken **token_carrier) {
  if (last_token(current_token)) return LAST_TOKEN;
  MiniStatus status;
  MiniToken *current_holder = current_token;

  current_token = next_token(current_token, &status);
  if (status != SUCCESS) return status;

  MiniSyntaxTree *new_node;
  MiniTokenCat categories[] = {TYPE_KW, IDENTIFIER, LITERAL_KW, LITERAL, -1};
  MiniNonTerm corresp_nonterms[] = {FOR_LOOP, WHILE_LOOP, WHILE_LOOP, WHILE_LOOP, -1};
  MiniTokenCat match;
  new_node = match_cat_and_add_nonterm_node(current_node, current_token, categories, corresp_nonterms, CHILD, &match, &status);
  if (status == NONMATCHING_CATEGORY) {
    printf("Parse Error: Invalid loop-block: Missing type, identifier, literal keyword or literal\n");
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  current_node = new_node;

  if (match == TYPE_KW) {
    return for_loop(current_node, current_holder, token_carrier);
  } else {
    return while_loop(current_node, current_holder, token_carrier);
  }
  return PARSE_ERROR; 
}


// <case-block> ::= "#=" (<mini-ID> | <mini-const-ID> | <mini-ext-ID> | <C-ID> | <int-literal> | <default>) ":" <sequence> ("~#" | <case-block>) 

static MiniStatus case_block(MiniSyntaxTree *current_node, MiniToken *current_token, MiniToken **token_carrier) {
  if (last_token(current_token)) return LAST_TOKEN;
  MiniStatus status;
  MiniSyntaxTree *cur_node = current_node;
  MiniToken *cur_token = current_token;
  MiniSyntaxTree *new_node;
  MiniToken *after_token = NULL;
  MiniTokenName name = CASE;
  new_node = match_and_add_term_node(cur_node, cur_token, &name, CHILD, NULL, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid case-block: Missing %s\n", desc_token(CASE));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;
  
  cur_node = new_node;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  MiniTokenName names[] = {MINI_ID, MINI_CONST_ID, MINI_EXT_ID, C_ID, INT_LITERAL, DEFAULT, -1};
  MiniTokenName match;
  new_node = match_and_add_term_node(cur_node, cur_token, names, SIBLING, &match, &status);
  MiniTokenName match_keeper = match;
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid case-block: Case value must reduce to a constant\n");
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;
 
  name = COLON;
  new_node = match_and_add_term_node(cur_node, cur_token, &name, SIBLING, NULL, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid case-block: Missing %s\n", desc_token(COLON));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;
  
  new_node = add_nonterm_node(cur_node, SEQUENCE, SIBLING, &status);
  if (status != SUCCESS) return status;

  cur_node = new_node;

  status = sequence(cur_node, cur_token, &after_token);
  if (status != VALID_CONSTRUCT) return status;

  cur_token = after_token;

  printf("DEBUG: cur token string repr: %s\n", cur_token->string_repr);

  MiniTokenName names2[] = {END_SWITCH, CASE, -1};
  if (match_keeper == DEFAULT) {
    names2[1] = -1;
  }
  status = match_terminals(cur_token, names2, &match);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid case-block: Missing %s or %s\n", desc_token(END_SWITCH), desc_token(CASE));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  if (match == CASE) {
    new_node = add_nonterm_node(cur_node, CASE_BLOCK, SIBLING, &status);
    if (status != SUCCESS) return status;

    cur_node = new_node;
    return case_block(cur_node, cur_token, token_carrier);
  } else if (match == END_SWITCH) {
    new_node = add_term_node(cur_node, cur_token, SIBLING, &status);
    if (status != SUCCESS) return status;

    cur_node = new_node;

    cur_token = next_token(cur_token, &status);
    if (status != SUCCESS) return status;

    *token_carrier = cur_token;
    return VALID_CONSTRUCT;
  }
  return PARSE_ERROR;
}

// <switch-block> ::= "##" <primary-expression> ":" (<sequence> | <case-block>)

static MiniStatus switch_block(MiniSyntaxTree *current_node, MiniToken *current_token, MiniToken **token_carrier) {
  if (last_token(current_token)) return LAST_TOKEN;
  MiniStatus status;
  MiniSyntaxTree *cur_node = current_node;
  MiniToken *cur_token = current_token;
  MiniSyntaxTree *new_node;
  MiniToken *after_token = NULL;
  MiniTokenName name = SWITCH;
  new_node = match_and_add_term_node(cur_node, cur_token, &name, CHILD, NULL, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid switch-block: Missing %s\n", desc_token(SWITCH));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;
  
  cur_node = new_node;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  new_node = add_nonterm_node(cur_node, PRIMARY_EXPRESSION, SIBLING, &status);
  if (status != SUCCESS) return status;

  cur_node = new_node;

  status = primary_expression(cur_node, cur_token, &after_token);
  if (status != VALID_CONSTRUCT) return status;

  cur_token = after_token;

  name = COLON;
  new_node = match_and_add_term_node(cur_node, cur_token, &name, SIBLING, NULL, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid switch-block: Missing %s\n", desc_token(COLON));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  name = CASE;
  status = match_terminals(cur_token, &name, NULL);
  if (status == NONMATCHING_TOKEN) {
    new_node = add_nonterm_node(cur_node, SEQUENCE, SIBLING, &status);
    if (status != SUCCESS) return status;

    cur_node = new_node;

    status = sequence(cur_node, cur_token, &after_token);
    if (status != VALID_CONSTRUCT) return status;

    cur_token = after_token;
  } else if (status != SUCCESS) return status; 

  new_node = add_nonterm_node(cur_node, CASE_BLOCK, SIBLING, &status);
  if (status != SUCCESS) return status;

  cur_node = new_node;
  return case_block(cur_node, cur_token, token_carrier);
}

// <logical-expression> ::= 

static MiniStatus logical_expression(MiniSyntaxTree *current_node, MiniToken *current_token, MiniToken **token_carrier) {
  if (last_token(current_token)) return LAST_TOKEN;

  MiniStatus status;
  MiniSyntaxTree *cur_node = current_node;
  MiniToken *cur_token = current_token;
  MiniSyntaxTree *new_node;
  //MiniToken *after_token = NULL;
  MiniTokenName names[] = {TRUE, FALSE, NUL, -1};
  MiniTokenName match;
  new_node = match_and_add_term_node(cur_node, cur_token, names, CHILD, &match, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("TEMPORARY: Parse Error: Invalid logical expression: Missing %s, %s or %s\n", desc_token(TRUE), desc_token(FALSE), desc_token(NUL));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  *token_carrier = cur_token;
  return VALID_CONSTRUCT;
}

// <else-block> ::= "|." ":" <sequence> "~?"

static MiniStatus else_block(MiniSyntaxTree *current_node, MiniToken *current_token, MiniToken **token_carrier) {
  if (last_token(current_token)) return LAST_TOKEN;

  MiniStatus status;
  MiniSyntaxTree *cur_node = current_node;
  MiniToken *cur_token = current_token;
  MiniSyntaxTree *new_node;
  MiniToken *after_token = NULL;
  MiniTokenName names[] = {ELSE, COLON, -1};
  MiniRelation rels[] = {CHILD, SIBLING, -1};
  MiniTokenName match;
  new_node = match_and_add_term_node_seq(cur_node, cur_token, &after_token, names, rels, &match, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid else-block: Missing %s\n", desc_token(match));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;
  
  cur_node = new_node;
  cur_token = after_token;

  new_node = add_nonterm_node(cur_node, SEQUENCE, SIBLING, &status);
  if (status != SUCCESS) return status;

  cur_node = new_node;

  status = sequence(cur_node, cur_token, &after_token);
  if (status != VALID_CONSTRUCT) return status;

  cur_token = after_token;
  
  MiniTokenName name = END_IF;
  new_node = match_and_add_term_node(cur_node, cur_token, &name, SIBLING, NULL, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid else-block: Missing %s\n", desc_token(END_IF));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  *token_carrier = cur_token;
  return VALID_CONSTRUCT;
}

// <else-if-block> ::= "|?" <logical-expression> ":" <sequence> ("~?" | <else-if-block> | <else-block>)

static MiniStatus elif_block(MiniSyntaxTree *current_node, MiniToken *current_token, MiniToken **token_carrier) {
  if (last_token(current_token)) return LAST_TOKEN;

  MiniStatus status;
  MiniSyntaxTree *cur_node = current_node;
  MiniToken *cur_token = current_token;
  MiniSyntaxTree *new_node;
  MiniToken *after_token = NULL;
  MiniTokenName name = ELSE_IF;
  new_node = match_and_add_term_node(cur_node, cur_token, &name, CHILD, NULL, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid else-if-block: Missing %s\n", desc_token(ELSE_IF));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;
  
  cur_node = new_node;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  new_node = add_nonterm_node(cur_node, LOGICAL_EXPR, SIBLING, &status);
  if (status != SUCCESS) return status;

  cur_node = new_node;

  status = logical_expression(cur_node, cur_token, &after_token);
  if (status != VALID_CONSTRUCT) return status;

  cur_token = after_token;

  name = COLON;
  new_node = match_and_add_term_node(cur_node, cur_token, &name, SIBLING, NULL, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid else-if-block: Missing %s\n", desc_token(COLON));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  new_node = add_nonterm_node(cur_node, SEQUENCE, SIBLING, &status);
  if (status != SUCCESS) return status;

  cur_node = new_node;

  status = sequence(cur_node, cur_token, &after_token);
  if (status != VALID_CONSTRUCT) return status;

  cur_token = after_token;

  MiniTokenName names[] = {END_IF, ELSE_IF, ELSE, -1};
  MiniTokenName match;
  status = match_terminals(cur_token, names, &match);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid else-if-block: Missing %s, %s or %s\n", desc_token(END_IF), desc_token(ELSE_IF), desc_token(ELSE));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  if (match == END_IF) {
    new_node = add_term_node(cur_node, cur_token, SIBLING, &status);
    if (status != SUCCESS) return status;

    cur_node = new_node;

    cur_token = next_token(cur_token, &status);
    if (status != SUCCESS) return status;

    *token_carrier = cur_token;
    return VALID_CONSTRUCT;
  } else if (match == ELSE_IF) {
    new_node = add_nonterm_node(cur_node, ELIF_BLOCK, SIBLING, &status);
    if (status != SUCCESS) return status;

    cur_node = new_node;

    return elif_block(cur_node, cur_token, token_carrier);
  } else if (match == ELSE) {
    new_node = add_nonterm_node(cur_node, ELSE_BLOCK, SIBLING, &status);
    if (status != SUCCESS) return status;

    cur_node = new_node;

    return else_block(cur_node, cur_token, token_carrier);
  }
  return PARSE_ERROR;
}


// <if-block> ::= "??" <logical-expression> ":" <sequence> ("~?" | <else-if-block> | <else-block>)

static MiniStatus if_block(MiniSyntaxTree *current_node, MiniToken *current_token, MiniToken **token_carrier) {
  if (last_token(current_token)) return LAST_TOKEN;

  MiniStatus status;
  MiniSyntaxTree *cur_node = current_node;
  MiniToken *cur_token = current_token;
  MiniSyntaxTree *new_node;
  MiniToken *after_token = NULL;
  MiniTokenName name = IF;
  new_node = match_and_add_term_node(cur_node, cur_token, &name, CHILD, NULL, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid if-block: Missing %s\n", desc_token(IF));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;
  
  cur_node = new_node;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  new_node = add_nonterm_node(cur_node, LOGICAL_EXPR, SIBLING, &status);
  if (status != SUCCESS) return status;

  cur_node = new_node;

  status = logical_expression(cur_node, cur_token, &after_token);
  if (status != VALID_CONSTRUCT) return status;

  cur_token = after_token;

  name = COLON;
  new_node = match_and_add_term_node(cur_node, cur_token, &name, SIBLING, NULL, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid if-block: Missing %s\n", desc_token(COLON));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  new_node = add_nonterm_node(cur_node, SEQUENCE, SIBLING, &status);
  if (status != SUCCESS) return status;

  cur_node = new_node;

  status = sequence(cur_node, cur_token, &after_token);
  if (status != VALID_CONSTRUCT) return status;

  cur_token = after_token;

  MiniTokenName names[] = {END_IF, ELSE_IF, ELSE, -1};
  MiniTokenName match;
  status = match_terminals(cur_token, names, &match);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid if-block: Missing %s, %s or %s\n", desc_token(END_IF), desc_token(ELSE_IF), desc_token(ELSE));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  if (match == END_IF) {
    new_node = add_term_node(cur_node, cur_token, SIBLING, &status);
    if (status != SUCCESS) return status;

    cur_node = new_node;

    cur_token = next_token(cur_token, &status);
    if (status != SUCCESS) return status;

    *token_carrier = cur_token;
    return VALID_CONSTRUCT;
  } else if (match == ELSE_IF) {
    new_node = add_nonterm_node(cur_node, ELIF_BLOCK, SIBLING, &status);
    if (status != SUCCESS) return status;

    cur_node = new_node;

    return elif_block(cur_node, cur_token, token_carrier);
  } else if (match == ELSE) {
    new_node = add_nonterm_node(cur_node, ELSE_BLOCK, SIBLING, &status);
    if (status != SUCCESS) return status;

    cur_node = new_node;

    return else_block(cur_node, cur_token, token_carrier);
  }
  return PARSE_ERROR;
}

// <branch> ::= <if-block> | <switch-block> | <loop>

static MiniStatus branch(MiniSyntaxTree *current_node, MiniToken *current_token, MiniToken **token_carrier) {
  if (last_token(current_token)) return LAST_TOKEN;

  MiniStatus status;
  MiniSyntaxTree *new_node;
  MiniTokenName names[] = {IF, SWITCH, LOOP, -1};
  MiniNonTerm corresp_nonterms[] = {IF_BLOCK, SWITCH_BLOCK, LOOP_BLOCK, -1};
  MiniTokenName match;
  new_node = match_and_add_nonterm_node(current_node, current_token, names, corresp_nonterms, CHILD, &match, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid branch: Missing %s, %s or %s\n", desc_token(IF), desc_token(SWITCH), desc_token(LOOP));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  current_node = new_node;

  if (match == IF) {
    return if_block(current_node, current_token, token_carrier);
  } else if (match == SWITCH) {
    return switch_block(current_node, current_token, token_carrier);
  } else if (match == LOOP) {
    return loop_block(current_node, current_token, token_carrier);
  }
  return PARSE_ERROR;
}


// <arg-list> ::= <primary-expression> ("" | "," <arg-list>)

static MiniStatus argument_list(MiniSyntaxTree *current_node, MiniToken *current_token, MiniToken **token_carrier) {
  if (last_token(current_token)) return LAST_TOKEN;

  MiniStatus status;
  MiniSyntaxTree *cur_node = current_node;
  MiniToken *cur_token = current_token;
  MiniSyntaxTree *new_node;
  MiniToken *after_token = NULL;
  new_node = add_nonterm_node(cur_node, PRIMARY_EXPRESSION, CHILD, &status);
  if (status != SUCCESS) return status;

  cur_node = new_node;

  status = primary_expression(cur_node, cur_token, &after_token);
  if (status != VALID_CONSTRUCT) return status;

  cur_token = after_token;

  MiniTokenName name = COMMA;
  new_node = match_and_add_term_node(cur_node, cur_token, &name, SIBLING, NULL, &status);
  if (status == NONMATCHING_TOKEN) {
    *token_carrier = cur_token;
    return VALID_CONSTRUCT;

  } else if (status != SUCCESS) return status;

  cur_node = new_node;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  new_node = add_nonterm_node(cur_node, ARGUMENT_LIST, SIBLING, &status);
  if (status != SUCCESS) return status;

  cur_node = new_node;

  return argument_list(cur_node, cur_token, token_carrier);
}

// <func-call> ::= "$" (<mini-ID> | <mini-ext-ID> | <C-ID) "(" <arg-list> ")"

static MiniStatus function_call(MiniSyntaxTree *current_node, MiniToken *current_token, MiniToken **token_carrier) {
  if (last_token(current_token)) return LAST_TOKEN;

  MiniStatus status;
  MiniSyntaxTree *cur_node = current_node;
  MiniToken *cur_token = current_token;
  MiniToken *after_token = NULL;
  MiniSyntaxTree *new_node;
  MiniTokenName name = CALL;
  new_node = match_and_add_term_node(cur_node, cur_token, &name, CHILD, NULL, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid function call: Missing %s\n", desc_token(CALL));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;
 
  cur_node = new_node;
  
  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  MiniTokenName names[] = {MINI_ID, MINI_EXT_ID, C_ID, -1};
  MiniTokenName match;
  new_node = match_and_add_term_node(cur_node, cur_token, names, SIBLING, &match, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid function call: Missing function name\n");
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  name = LEFT_PAREN;
  new_node = match_and_add_term_node(cur_node, cur_token, &name, SIBLING, NULL, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid function call: Missing %s\n", desc_token(LEFT_PAREN));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;
 
  cur_node = new_node;
  
  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  name = RIGHT_PAREN;
  new_node = match_and_add_term_node(cur_node, cur_token, &name, SIBLING, NULL, &status);
  if (status == SUCCESS) {
    cur_node = new_node;

    cur_token = next_token(cur_token, &status);
    if (status != SUCCESS) return status;

    *token_carrier = cur_token;
    return VALID_CONSTRUCT;

  } else if (status != NONMATCHING_TOKEN) return status;
 
  new_node = add_nonterm_node(cur_node, ARGUMENT_LIST, CHILD, &status);
  if (status != SUCCESS) return status;

  cur_node = new_node;

  status = argument_list(cur_node, cur_token, &after_token);
  if (status != VALID_CONSTRUCT) return status;

  cur_token = after_token;

  new_node = match_and_add_term_node(cur_node, cur_token, &name, SIBLING, NULL, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid function call: Missing %s\n", desc_token(RIGHT_PAREN));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  *token_carrier = cur_token;
  return VALID_CONSTRUCT;
}

// <flow-control> ::= "." | ".." | "<-" <primary-expression>

static MiniStatus flow_control(MiniSyntaxTree *current_node, MiniToken *current_token, MiniToken **token_carrier) {
  if (last_token(current_token)) return LAST_TOKEN;

  MiniStatus status;
  MiniSyntaxTree *new_node;
  MiniTokenName names[] = {BREAK, CONTINUE, RETURN, -1};
  MiniTokenName match;
  new_node = match_and_add_term_node(current_node, current_token, names, CHILD, &match, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid flow control statement: Missing %s, %s or %s\n", desc_token(BREAK), desc_token(CONTINUE), desc_token(RETURN));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  current_node = new_node;

  current_token = next_token(current_token, &status);
  if (status != SUCCESS) return status;

  if (match == RETURN) {
    new_node = add_nonterm_node(current_node, PRIMARY_EXPRESSION, SIBLING, &status);
    if (status != SUCCESS) return status;

    current_node = new_node;

    return primary_expression(current_node, current_token, token_carrier);
  }

  *token_carrier = current_token;
  return VALID_CONSTRUCT;
}

// <io-control> ::= "!" ("..." | <mini-ID> | <mini-const-ID> | <mini-ext-ID> | <C-ID> | <string> ) "->"
//                  ("..." | <mini-ID> | <mini-ext-ID> | <C-ID>)

static MiniStatus input_output_control(MiniSyntaxTree *current_node, MiniToken *current_token, MiniToken **token_carrier) {
  if (last_token(current_token)) return LAST_TOKEN;

  MiniStatus status;
  MiniSyntaxTree *cur_node = current_node;
  MiniToken *cur_token = current_token;
  MiniSyntaxTree *new_node = NULL;
  MiniTokenName name = READ_WRITE;
  new_node = match_and_add_term_node(cur_node, cur_token, &name, CHILD, NULL, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid input/output statement: Missing %s\n", desc_token(READ_WRITE));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  MiniTokenName names[] = {STDIO, MINI_ID, MINI_CONST_ID, MINI_EXT_ID, C_ID, STRING_LITERAL, -1};
  MiniTokenName match;
  new_node = match_and_add_term_node(cur_node, cur_token, names, SIBLING, &match, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid input/output statement: Missing source for reading/writing\n");
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  name = REDIRECT;
  new_node = match_and_add_term_node(cur_node, cur_token, &name, SIBLING, NULL, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid input/output statement: Missing %s\n", desc_token(REDIRECT));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  MiniTokenName names2[] = {STDIO, MINI_ID, MINI_EXT_ID, C_ID, -1};
  new_node = match_and_add_term_node(cur_node, cur_token, names2, SIBLING, &match, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid input/output statement: Missing destination for reading/writing\n");
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  *token_carrier = cur_token;
  return VALID_CONSTRUCT;
}

// <control> ::= <io-control> | <flow-control> | <func-call>

static MiniStatus control(MiniSyntaxTree *current_node, MiniToken *current_token, MiniToken **token_carrier) {
  if (last_token(current_token)) return LAST_TOKEN;

  MiniStatus status;
  MiniSyntaxTree *new_node;
  MiniTokenName names[] = {READ_WRITE, CALL, RETURN, BREAK, CONTINUE, -1};
  MiniNonTerm corresp_nonterms[] = {IN_OUT_CTRL, FUNC_CALL, FLOW_CTRL, FLOW_CTRL, FLOW_CTRL, -1};
  MiniTokenName match;
  new_node = match_and_add_nonterm_node(current_node, current_token, names, corresp_nonterms, CHILD, &match, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid control statement: Missing %s, %s, %s, %s or %s\n", desc_token(READ_WRITE), desc_token(CALL), desc_token(RETURN), desc_token(BREAK), desc_token(CONTINUE));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  current_node = new_node;

  if (match == READ_WRITE) {
    return input_output_control(current_node, current_token, token_carrier);
  } else if (match == CALL) {
    return function_call(current_node, current_token, token_carrier);
  }

  return flow_control(current_node, current_token, token_carrier);
}

// <incrementation> ::= ((<mini-id> | <mini-ext-id> | <C-id>) (<BIN-A-OP> <expression> | <UNA-A-OP>)) 
//                      | <UNA-A-OP> (<mini-id> | <mini-ext-id> | <C-id>)

static MiniStatus incrementation(MiniSyntaxTree *current_node, MiniToken *current_token, MiniToken **token_carrier) {
  if (last_token(current_token)) return LAST_TOKEN;

  MiniStatus status;
  MiniSyntaxTree *cur_node = current_node;
  MiniToken *cur_token = current_token;
  //MiniToken *after_token = NULL;
  MiniSyntaxTree *new_node;
  MiniTokenName names[] = {MINI_ID, MINI_EXT_ID, C_ID, INCREMENT, DECREMENT, -1};
  // Could also be done with MiniTokenCat categories[] = {IDENTIFIER, UNA_ASSIGN_OP, -1};
  // but then would have to manually add the matching token
  MiniTokenName match;
  new_node = match_and_add_term_node(cur_node, cur_token, names, CHILD, &match, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid incrementation: Missing identifier or increment/decrement operator\n");
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  if (match == INCREMENT || match == DECREMENT) {
    MiniTokenName names2[] = {MINI_ID, MINI_CONST_ID, C_ID, -1};
    new_node = match_and_add_term_node(cur_node, cur_token, names2, SIBLING, &match, &status);
    if (status == NONMATCHING_TOKEN) {
      printf("Parse Error: Invalid incrementation: Missing identifier after increment/decrement operator\n");
      return PARSE_ERROR;
    } else if (status != SUCCESS) return status;

    cur_node = new_node;

    cur_token = next_token(cur_token, &status);
    if (status != SUCCESS) return status;

    *token_carrier = cur_token;
    return VALID_CONSTRUCT;
  }

  MiniTokenName names3[] = {PLUS_ASSIGN, MINUS_ASSIGN, TIMES_ASSIGN, DIV_ASSIGN,MOD_ASSIGN,
                            INCREMENT, DECREMENT, -1};
  // Could also be done with MiniTokenCat categories = {BIN_ASSIGN_OP, UNA_ASSIGN_OP, -1}
  // but then would have to manually add the matching token
  new_node = match_and_add_term_node(cur_node, cur_token, names3, SIBLING, &match, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid incrementation: Missing reassignment/increment/decrement operator\n");
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  if (match == INCREMENT || match == DECREMENT) {
    *token_carrier = cur_token;
    return VALID_CONSTRUCT;
  }

  new_node = add_nonterm_node(cur_node, PRIMARY_EXPRESSION, SIBLING, &status);
  if (status != SUCCESS) return status;

  cur_node = new_node;

  return primary_expression(cur_node, cur_token, token_carrier);
}

// <assignment> ::= (<mini-id> | <mini-ext-id> | <C-id>) ":=" (<primary-expression> | <collection>)

static MiniStatus assignment(MiniSyntaxTree *current_node, MiniToken *current_token, MiniToken **token_carrier) {
  if (last_token(current_token)) return LAST_TOKEN;

  MiniStatus status;
  MiniSyntaxTree *cur_node = current_node;
  MiniToken *cur_token = current_token;
  //MiniToken *after_token = NULL;
  MiniSyntaxTree *new_node;
  MiniTokenName names[] = {MINI_ID, MINI_EXT_ID, C_ID, -1};
  MiniTokenName match;
  new_node = match_and_add_term_node(cur_node, cur_token, names, CHILD, &match, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid assignment specification: Missing %s, %s or %s\n", desc_token(MINI_ID), desc_token(MINI_EXT_ID), desc_token(C_ID));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  MiniTokenName name = ASSIGN;
  new_node = match_and_add_term_node(cur_node, cur_token, &name, SIBLING, NULL, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid Assignment specification: Missing %s\n", desc_token(ASSIGN));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  names[0] = LEFT_BRACKET;
  names[1] = LEFT_BRACE;
  status = match_terminals(cur_token, names, &match);
  if (status == NONMATCHING_TOKEN) {
    new_node = add_nonterm_node(cur_node, PRIMARY_EXPRESSION, SIBLING, &status);
    if (status != SUCCESS) return status;

    cur_node = new_node;
    return primary_expression(cur_node, cur_token, token_carrier);
  } else if (status != SUCCESS) return status;

  new_node = add_nonterm_node(cur_node, COLLECTION, SIBLING, &status);
  if (status != SUCCESS) return status;

  cur_node = new_node;
  return collection(cur_node, cur_token, token_carrier);
}

// <designation> ::= <assignment> | <incrementation>

static MiniStatus designation(MiniSyntaxTree *current_node, MiniToken *current_token, MiniToken **token_carrier) {
  if (last_token(current_token)) return LAST_TOKEN;

  MiniStatus status;
  MiniSyntaxTree *new_node;
  MiniToken *current_holder = current_token;

  current_token = next_token(current_token, &status);
  if (status != SUCCESS) return status;

  MiniTokenName name = ASSIGN;
  status = match_terminals(current_token, &name, NULL);
  if (status == NONMATCHING_TOKEN) {
    new_node = add_nonterm_node(current_node, INCREMENTATION, CHILD, &status);
    if (status != SUCCESS) return status;

    current_node = new_node;

    return incrementation(current_node, current_holder, token_carrier);
  } else if (status != SUCCESS) return status;

  new_node = add_nonterm_node(current_node, ASSIGNMENT, CHILD, &status);
  if (status != SUCCESS) return status;

  current_node = new_node;
 
  return assignment(current_node, current_holder, token_carrier);
}

// <statement> ::= (<declaration> | <designation> | <contol>) ";"

static MiniStatus statement(MiniSyntaxTree *current_node, MiniToken *current_token, MiniToken **token_carrier) {
  if (last_token(current_token)) return LAST_TOKEN;

  MiniStatus status;
  MiniSyntaxTree *cur_node = current_node;
  MiniToken *cur_token = current_token;
  MiniToken *after_token = NULL;
  MiniSyntaxTree *new_node;
  MiniTokenCat categories[] = {TYPE_KW, CONTROL_KW, -1};
  MiniNonTerm corresp_nonterms[] = {DECLARATION, CONTROL, -1};
  MiniTokenCat match;
  new_node = match_cat_and_add_nonterm_node(cur_node, cur_token, categories, corresp_nonterms, CHILD, &match, &status);
  if (status == NONMATCHING_CATEGORY) {
    new_node = add_nonterm_node(cur_node, DESIGNATION, CHILD, &status);
    if (status != SUCCESS) return status;

    cur_node = new_node;

    status = designation(cur_node, cur_token, &after_token);
  } else if (status != SUCCESS) {
    return status;
  } else {
    cur_node = new_node;

    if (match == TYPE_KW) {
      status = declaration(cur_node, cur_token, &after_token);
    } else if (match == CONTROL_KW) {
      status = control(cur_node, cur_token, &after_token);
    }
  }
  if (status != VALID_CONSTRUCT) return status;

  cur_token = after_token;

  MiniTokenName name = SEMICOLON;
  new_node = match_and_add_term_node(cur_node, cur_token, &name, SIBLING, NULL, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid statement: Missing %s\n", desc_token(SEMICOLON));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  *token_carrier = cur_token;
  return VALID_CONSTRUCT;
  return VALID_CONSTRUCT;
}

// <sequence> ::= (<statement> | <branch>) ("" | <sequence>)

static MiniStatus sequence(MiniSyntaxTree *current_node, MiniToken *current_token, MiniToken **token_carrier) {
  if (last_token(current_token)) return LAST_TOKEN;

  MiniStatus status;
  MiniSyntaxTree *cur_node = current_node;
  MiniToken *cur_token = current_token;
  MiniToken *after_token = NULL;
  MiniSyntaxTree *new_node;
  MiniTokenCat category = BRANCH_KW;
  MiniNonTerm corresp_nonterm = BRANCH;
  new_node = match_cat_and_add_nonterm_node(cur_node, cur_token, &category, &corresp_nonterm, CHILD, NULL, &status);
  if (status == NONMATCHING_CATEGORY) {
    new_node = add_nonterm_node(cur_node, STATEMENT, CHILD, &status);
    if (status != SUCCESS) return status;

    cur_node = new_node;

    status = statement(cur_node, cur_token, &after_token);
    if (status != VALID_CONSTRUCT) return status;

  } else if (status != SUCCESS) {
    return status;
  } else {
    cur_node = new_node;

    status = branch(cur_node, cur_token, &after_token);
    if (status != VALID_CONSTRUCT) return status;
  }

  cur_token = after_token;

  category = TERM_KW;
  status = match_terminal_cats(cur_token, &category, NULL);
  if (status == NONMATCHING_CATEGORY) {
    new_node = add_nonterm_node(cur_node, SEQUENCE, SIBLING, &status);
    if (status != SUCCESS) return status;

    cur_node = new_node;
  
    return sequence(cur_node, cur_token, token_carrier);

  } else if (status != SUCCESS) return status;

  *token_carrier = cur_token;
  return VALID_CONSTRUCT;
}

// <main-part> ::= ">>>" <mini-id> ("[..]" | "") ":" <sequence> "<<<" 

static MiniStatus main_part(MiniSyntaxTree *current_node, MiniToken *current_token) {
  if (last_token(current_token)) return LAST_TOKEN;

  MiniStatus status;
  MiniSyntaxTree *cur_node = current_node;
  MiniSyntaxTree *new_node;
  MiniToken *after_token;
  MiniTokenName non_match;
  MiniTokenName names[] = {MAIN, MINI_ID, -1};
  MiniRelation rels[] = {CHILD, SIBLING, -1};
  
  new_node = match_and_add_term_node_seq(cur_node, current_token, &after_token, names, rels, &non_match, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid main part specification: Missing %s\n", desc_token(non_match));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;
  cur_node = new_node;

  MiniTokenName names2[] = {ARGV, COLON, -1};
  MiniTokenName match;
  new_node = match_and_add_term_node(cur_node, after_token, names2, SIBLING, &match, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid main part specification: Missing %s\n", desc_token(COLON));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;
  after_token = next_token(after_token, &status);
  if (status != SUCCESS) return status;

  if (match == ARGV) {
    MiniTokenName name = COLON;
    new_node = match_and_add_term_node(cur_node, after_token, &name, SIBLING, NULL, &status);
    if (status == NONMATCHING_TOKEN) {
      printf("Parse Error: Invalid main part specification: Missing %s\n", desc_token(COLON));
      return PARSE_ERROR;
    } else if (status != SUCCESS) return status;
    cur_node = new_node;
    
    after_token = next_token(after_token, &status);
    if (status != SUCCESS) return status;
  }

  new_node = add_nonterm_node(cur_node, SEQUENCE, SIBLING, &status);
  if (status != SUCCESS) return status;
  cur_node = new_node;

  MiniToken *after_token2 = NULL;
  status = sequence(cur_node, after_token, &after_token2);
  if (status != VALID_CONSTRUCT) return status;

  MiniTokenName name = END_MAIN;
  new_node = match_and_add_term_node(cur_node, after_token2, &name, SIBLING, NULL, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid main part specification: Missing %s\n", desc_token(END_MAIN));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  after_token2 = next_token(after_token2, &status);
  if (status != LAST_TOKEN) {
    printf("Parse Error: Extra token(s) following end of main part\n");
    return PARSE_ERROR;
  }

  return VALID_CONSTRUCT;
}

// XXX: Left off here
// <arith-operand> ::= <mini-ID> | <mini-const-ID> | <mini-ext-ID> | <C-id> | <int-lit> | <float-lit> | <func-call> | "(" <arithmetic-expr> ")"


// <arithmetic-expr> ::= <arith-operand> <bin-arith-oper> <arith-operand> | <una-arith-oper> <arith-operand> | <arith-operand>
//                       | <indexing> | <sizeof>


// <logical-expr> ::= <logical-operand> <bin-log-oper> <logical-operand> 

// <expression> ::= <arithmetic-expr> | <logical-expr> | "(" <primary-expression> ")"

static MiniStatus expression(MiniSyntaxTree *current_node, MiniToken *current_token, MiniToken **token_carrier) {
  if (last_token(current_token)) return LAST_TOKEN;

  MiniStatus status;
  MiniSyntaxTree *cur_node = current_node;
  MiniToken *cur_token = current_token;
  MiniSyntaxTree *new_node;
  MiniToken *after_token = NULL;

  MiniTokenName name = LEFT_PAREN;
  new_node = match_and_add_term_node(cur_node, cur_token, &name, CHILD, NULL, &status);
  if (status == NONMATCHING_TOKEN) {
    bool logical_expr = false;
    MiniToken *temp_token = cur_token;
    status = SUCCESS;
    while (status == SUCCESS && temp_token->category != PUNCT_SEP) {
      MiniTokenCat cur_cat = temp_token->category;
      if (cur_cat == COMP_OP || cur_cat == BIN_LOG_OP || cur_cat == UNA_LOG_OP) {
        logical_expr = true;
        break;
      } 
      temp_token = next_token(temp_token, &status);
    }    

    if (status != SUCCESS) return status;

    if (logical_expr) {
      new_node = add_nonterm_node(cur_node, LOGICAL_EXPR, CHILD, &status);
      if (status != SUCCESS) return status;
      
      cur_node = new_node;

      return logical_expression(cur_node, cur_token, token_carrier);
    } else {
      new_node = add_nonterm_node(cur_node, ARITHMETIC_EXPR, CHILD, &status);
      if (status != SUCCESS) return status;

      cur_node = new_node;

      return arithmetic_expression(cur_node, cur_token, token_carrier);
    }
  } else if (status != SUCCESS) return status;
  
  cur_node = new_node;
  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  new_node = add_nonterm_node(cur_node, PRIMARY_EXPRESSION, SIBLING, &status);
  if (status != SUCCESS) return status;

  cur_node = new_node;
  status = primary_expression(cur_node, cur_token, &after_token);
  if (status != SUCCESS) return status;

  cur_token = after_token;

  name = RIGHT_PAREN;
  new_node = match_and_add_term_node(cur_node, cur_token, &name, SIBLING, NULL, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid Expression: Missing %s\n", desc_token(RIGHT_PAREN));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;
  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  *token_carrier = cur_token;
  return VALID_CONSTRUCT;  
}

// <primary-expression> ::= <mini-ID> | <mini-const-ID> | <mini-ext-ID> | <C-ID> | <int-lit> | <float-lit>
//                          | <str-lit> | <kw-lit> | <expression>

static MiniStatus primary_expression(MiniSyntaxTree *current_node, MiniToken *current_token, MiniToken **token_carrier) {
  if (last_token(current_token)) return LAST_TOKEN;

  MiniStatus status;
  MiniSyntaxTree *cur_node = current_node;
  MiniToken *cur_token = current_token;
  MiniSyntaxTree *new_node;

  MiniToken *temp_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  MiniToken names[] = {SEMICOLON, COLON, -1};
  MiniToken match;
  status = match_terminals(temp_token, names, &match);
  if (status == NONMATCHING_TOKEN) {
    new_node = add_nonterm_node(cur_node, EXPRESSION, CHILD, &status);
    if (status != SUCCESS) return status;

    cur_node = new_node;

    return expression(cur_node, cur_token, token_carrier);
  } else if (status != SUCCESS) return status;

  MiniTokenName names2[] = {MINI_ID, MINI_CONST_ID, MINI_EXT_ID, C_ID, INT_LITERAL, FLOAT_LITERAL, STRING_LITERAL, TRUE, FALSE, NUL, -1};
  new_node = match_and_add_term_node(cur_node, cur_token, names2, CHILD, &match, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid primary expression: Missing identifier, literal or keyword literal\n");
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;
  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  *token_carrier = cur_token;
  return VALID_CONSTRUCT;
}

// <dict> ::= (<literal> | <mini-const-id>) ":" (<literal> | <true> | <false> | <mini-const-id>) "," <dict> | (<literal> | <mini-const-id>) ":" (<literal> | <true> | <false> | <mini-const-id>)

static MiniStatus dictionary(MiniSyntaxTree *current_node, MiniToken *current_token, MiniToken **token_carrier) {
  if (last_token(current_token)) return LAST_TOKEN;

  MiniToken *cur_token = current_token;
  MiniStatus status;

  MiniSyntaxTree *new_node;
  MiniSyntaxTree *cur_node = current_node;
  MiniTokenName names[] = {INT_LITERAL, FLOAT_LITERAL, STRING_LITERAL, TRUE, FALSE, MINI_CONST_ID, -1};
  MiniTokenName match;
  new_node = match_and_add_term_node(cur_node, cur_token, names, CHILD, &match, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid dictionary: %s is not a valid dictionary key\nNote: Dictionary key must be %s, %s, %s, %s, %s or %s\n", cur_token->string_repr, desc_token(INT_LITERAL), desc_token(FLOAT_LITERAL), desc_token(STRING_LITERAL), desc_token(TRUE), desc_token(FALSE), desc_token(MINI_CONST_ID));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  MiniTokenName name = COLON;
  new_node = match_and_add_term_node(cur_node, cur_token, &name, SIBLING, NULL, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid dictionary: Missing %s\n", desc_token(COLON));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;
  
  cur_node = new_node;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  MiniTokenName names2[] = {INT_LITERAL, FLOAT_LITERAL, STRING_LITERAL, MINI_CONST_ID, -1};
  new_node = match_and_add_term_node(cur_node, cur_token, names2, SIBLING, &match, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid dictionary: %s is not a valid dictionary value\nNote: Dictionary value must be %s, %s, %s or %s\n", cur_token->string_repr, desc_token(INT_LITERAL), desc_token(FLOAT_LITERAL), desc_token(STRING_LITERAL), desc_token(MINI_CONST_ID));
    return PARSE_ERROR;
  }

  cur_node = new_node;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  name = COMMA;
  new_node = match_and_add_term_node(cur_node, cur_token, &name, SIBLING, NULL, &status);
  if (status == NONMATCHING_TOKEN) {
    *token_carrier = cur_token;
    return VALID_CONSTRUCT;
  } else if (status != SUCCESS) return status;
  
  cur_node = new_node;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  new_node = add_nonterm_node(cur_node, DICT, SIBLING, &status);
  if (status != SUCCESS) return status;

  cur_node = new_node;

  return dictionary(cur_node, cur_token, token_carrier);
}

// <list> ::= (<literal> | <true> | <false> | <mini-const-id>) "," <list> | <literal> | <mini-const-id>

static MiniStatus list(MiniSyntaxTree *current_node, MiniToken *current_token, MiniToken **token_carrier) {
  if (last_token(current_token)) return LAST_TOKEN;

  MiniToken *cur_token = current_token;
  MiniStatus status;

  MiniSyntaxTree *new_node;
  MiniSyntaxTree *cur_node = current_node;
  MiniTokenName names[] = {INT_LITERAL, FLOAT_LITERAL, STRING_LITERAL, TRUE, FALSE, MINI_CONST_ID, -1};
  MiniTokenName match;
  new_node = match_and_add_term_node(cur_node, cur_token, names, CHILD, &match, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid list: %s is not a valid list element\nNote: List element must be %s, %s, %s, %s, %s or %s\n", cur_token->string_repr, desc_token(INT_LITERAL), desc_token(FLOAT_LITERAL), desc_token(STRING_LITERAL), desc_token(TRUE), desc_token(FALSE), desc_token(MINI_CONST_ID));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  MiniTokenName name = COMMA;
  new_node = match_and_add_term_node(cur_node, cur_token, &name, SIBLING, NULL, &status);
  if (status == NONMATCHING_TOKEN) {
    *token_carrier = cur_token;
    return VALID_CONSTRUCT;
  } else if (status != SUCCESS) return status;
  
  cur_node = new_node;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  new_node = add_nonterm_node(cur_node, LIST, SIBLING, &status);
  if (status != SUCCESS) return status;

  cur_node = new_node;

  return list(cur_node, cur_token, token_carrier);
}

// <collection> ::= "[" (<list> | <dict>) "]"         (((| "{" (<enum> | <struct> | <union>) "}")))

static MiniStatus collection(MiniSyntaxTree *current_node, MiniToken *current_token, MiniToken **token_carrier) {
  if (last_token(current_token)) return LAST_TOKEN;

  MiniToken *cur_token = current_token;
  MiniStatus status;

  MiniSyntaxTree *new_node;
  MiniSyntaxTree *cur_node = current_node;
  MiniTokenName name = LEFT_BRACKET;
  new_node = match_and_add_term_node(cur_node, cur_token, &name, CHILD, NULL, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid collection: Missing %s\n", desc_token(LEFT_BRACKET));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;
 
  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  MiniToken current_holder = *cur_token;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  MiniToken *after_token = NULL;
  MiniTokenName names[] = {COMMA, COLON, -1};
  MiniTokenName match;
  status = match_terminals(cur_token, names, &match);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid collection: Missing %s or %s\n", desc_token(COMMA), desc_token(COLON));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  if (match == COMMA) {
    new_node = add_nonterm_node(cur_node, LIST, SIBLING, &status); 
    
    cur_node = new_node;
    status = list(cur_node, &current_holder, &after_token);
    
  } else if (match == COLON) {
    new_node = add_nonterm_node(cur_node, DICT, SIBLING, &status);
    
    cur_node = new_node;
    status = dictionary(cur_node, &current_holder, &after_token);
  }

  if (status != VALID_CONSTRUCT) return status;

  name = RIGHT_BRACKET;
  new_node = match_and_add_term_node(cur_node, after_token, &name, SIBLING, NULL, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid collection: Missing %s\n", desc_token(RIGHT_BRACKET));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;
  
  new_node = cur_node;

  after_token = next_token(after_token, &status);
  if (status != SUCCESS) return status;

  *token_carrier = after_token;
  return VALID_CONSTRUCT;
}

// <type> ::= "<>" | "<#>" | "<%>" | "<">" | "<B>" | "<S>" | "[]" | "[:]" | "{E}" | "{U}" | "{S}" | <custom>

static MiniStatus type(MiniSyntaxTree *current_node, MiniToken *current_token) {
  if (last_token(current_token)) return LAST_TOKEN;

  MiniStatus status;
  MiniTokenName names[] = {VOID, INT, FLOAT, STR, BOOL, STREAM, LIST_T, DICT_T, ENUM_T, UNION_T, STRUCT_T, CUSTOM_T, -1};
  MiniTokenName match;
  match_and_add_term_node(current_node, current_token, names, CHILD, &match, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid type: %s is not recognized as a type\n", current_token->string_repr);
    return PARSE_ERROR;
  }
  return VALID_CONSTRUCT;
}

// <param-list> ::= <type> <mini-id> ("," <param-list> | "")

static MiniStatus parameter_list(MiniSyntaxTree *current_node, MiniToken *current_token, MiniToken **token_carrier) {
  if (last_token(current_token)) return LAST_TOKEN;

  MiniStatus status;
  MiniSyntaxTree *cur_node = current_node;
  MiniToken *cur_token = current_token;
  MiniSyntaxTree *new_node;
  //MiniToken *after_token = NULL;
  MiniTokenCat category = TYPE_KW;
  MiniNonTerm corresp_nonterm = TYPE_EXPR;
  new_node = match_cat_and_add_nonterm_node(cur_node, cur_token, &category, &corresp_nonterm, CHILD, NULL, &status);
  if (status == NONMATCHING_CATEGORY) {
    printf("Parse Error: Invalid parameter list specification: Missing type keyword\n");
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;

  status = type(cur_node, cur_token);
  if (status != VALID_CONSTRUCT) return status;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  MiniTokenName name = MINI_ID;
  new_node = match_and_add_term_node(cur_node, cur_token, &name, SIBLING, NULL, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid parameter list specification: Missing %s\n", desc_token(MINI_ID));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  name = COMMA;
  new_node = match_and_add_term_node(cur_node, cur_token, &name, SIBLING, NULL, &status);
  if (status == NONMATCHING_TOKEN) {
    *token_carrier = cur_token;
    return VALID_CONSTRUCT;

  } else if (status != SUCCESS) return status;

  cur_node = new_node;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  corresp_nonterm = PARAM_LIST;
  new_node = match_cat_and_add_nonterm_node(cur_node, cur_token, &category, &corresp_nonterm, SIBLING, NULL, &status);
  if (status == NONMATCHING_CATEGORY) {
    printf("Parse Error: Invalid parameter list specification: Missing type keyword after comma\n");
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;
  
  return parameter_list(cur_node, cur_token, token_carrier);
}

// <subprogram> ::= "$$" <mini-id> "(" <param-list> ")" "->" <type> ":" <sequence> "~$"

static MiniStatus subprogram(MiniSyntaxTree *current_node, MiniToken *current_token, MiniToken **token_carrier) {
  if (last_token(current_token)) return LAST_TOKEN;

  MiniStatus status;
  MiniSyntaxTree *cur_node = current_node;
  MiniToken *cur_token = current_token;
  MiniSyntaxTree *new_node;
  MiniToken *after_token = NULL;
  MiniTokenName names[] = {FUNC, MINI_ID, LEFT_PAREN, -1};
  MiniRelation rels[] = {CHILD, SIBLING, SIBLING, -1};
  MiniTokenName match;
  new_node = match_and_add_term_node_seq(cur_node, cur_token, &after_token, names, rels, &match, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid subprogram specification: Missing %s\n", desc_token(match));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;
  cur_token = after_token;

  if (last_token(cur_token)) return LAST_TOKEN;

  MiniTokenCat category = TYPE_KW;
  MiniNonTerm corresp_nonterm = PARAM_LIST;
  new_node = match_cat_and_add_nonterm_node(cur_node, cur_token, &category, &corresp_nonterm, SIBLING, NULL, &status);
  if (status == SUCCESS) {
    new_node = cur_node;

    status = parameter_list(cur_node, cur_token, &after_token);
    if (status != VALID_CONSTRUCT) return status;

    cur_token = after_token;
  } else if (status != SUCCESS && status != NONMATCHING_CATEGORY) return status;

  MiniTokenName names2[] = {RIGHT_PAREN, REDIRECT, -1};
  MiniRelation rels2[] = {SIBLING, SIBLING, -1};
  new_node = match_and_add_term_node_seq(cur_node, cur_token, &after_token, names2, rels2, &match, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid subprogram specification: Missing %s\n", desc_token(match));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;
  cur_token = after_token;

  if (last_token(cur_token)) return LAST_TOKEN;
  
  corresp_nonterm = TYPE_EXPR;
  new_node = match_cat_and_add_nonterm_node(cur_node, cur_token, &category, &corresp_nonterm, SIBLING, NULL, &status);
  if (status == NONMATCHING_CATEGORY) {
    printf("Parse Error: Invalid subprogram specification: Missing return type\n");
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;
  status = type(cur_node, cur_token);
  if (status != VALID_CONSTRUCT) return status;

  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  MiniTokenName name = COLON;
  new_node = match_and_add_term_node(cur_node, cur_token, &name, SIBLING, NULL, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid subprogram specification: Missing %s\n", desc_token(COLON));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;
  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  new_node = add_nonterm_node(cur_node, SEQUENCE, SIBLING, &status);
  if (status != SUCCESS) return status;

  cur_node = new_node;
  status = sequence(cur_node, cur_token, &after_token);
  if (status != VALID_CONSTRUCT) return status;

  cur_token = after_token;
  
  name = END_FUNC;
  new_node = match_and_add_term_node(cur_node, cur_token, &name, SIBLING, NULL, &status);
  if (status != SUCCESS) {
    printf("Parse Error: Invalid subprgram specification: Missing %s\n", desc_token(END_FUNC));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;
  cur_token = next_token(cur_token, &status);
  if (status != SUCCESS) return status;

  *token_carrier = cur_token;
  return VALID_CONSTRUCT;
} 
/*
// <rvalue> ::= <literal> | <mini-id> | <mini-const-id> | <expression>

static right_value(MiniSyntaxTree *current_node, MiniToken *current_token, MiniToken **token_carrier) {
  if (last_token(current_token)) return LAST_TOKEN;

  MiniStatus status;
  MiniToken current_keeper = *current_token;
  MiniSyntaxTree *cur_node = current_node;
  MiniToken *new_node;

  current_token = next_token(current_token, &status);
  if (status != SUCCESS) return status;

  MiniTokenName name = SEMICOLON;
  status = match_terminals(current_token, &name, NULL);
  if (status == NONMATCHING_CATEGORY) {
    
    new_node = add_nonterm_node(cur_node, EXPRESSION, CHILD, &status);
    if (status != SUCCESS) return status;

    return expre

  } else if (status != SUCCESS) return status;

  MiniTokenName names[] = {INT_LITERAL, FLOAT_LITERAL, STRING_LITERAL, TRUE, FALSE, MINI_ID, MINI_CONST_ID, MINI_EXT_ID, -1};
  MiniTokenName match;
  new_node = match_and_add_term_node(cur_node, current_keeper, names, CHILD, &match, &status);
  if (status == NONMATCHING_TOKEN) {
    // XXX: Left off here
    printf("Parse Error: Invalid Rvalue: %s\n", current_token->string_repr);
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;
  current_keeper = next_node(current_keeper, &status);
  if (status != SUCCESS) return status;

  *token_carrier = current_keeper;
  return VALID_CONSTRUCT;
}
*/

// <declaration> ::= <type> <mini-id> ("" | ":=" (<collection> | <primary-expression>)) ";"

static MiniStatus module_declaration(MiniSyntaxTree *current_node, MiniToken *current_token, MiniToken **token_carrier) {
  if (last_token(current_token)) return LAST_TOKEN;

  MiniStatus status;
  //MiniToken *after_token = NULL;
  MiniSyntaxTree *cur_node = current_node;
  MiniSyntaxTree *new_node;
  MiniTokenCat category = TYPE_KW;
  MiniNonTerm corresp_nonterm = TYPE_EXPR;
  //MiniTokenCat match;
  new_node = match_cat_and_add_nonterm_node(cur_node, current_token, &category, &corresp_nonterm, CHILD, NULL, &status);
  if (status == NONMATCHING_CATEGORY) {
    printf("Parse Error: Invalid Module Declaration: Missing type keyword\n");
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;
  
  cur_node = new_node;
  status = type(cur_node, current_token);
  if (status != VALID_CONSTRUCT) return status;

  current_token = next_token(current_token, &status);
  if (status != SUCCESS) return status;

  MiniTokenName names[] = {MINI_ID, MINI_CONST_ID, -1};
  /*
  if (after_token) {
    new_node = match_and_add_term_node(cur_node, after_token, names, SIBLING, &match, &status);
  } else {
    new_node = match_and_add_term_node(cur_node, current_token, names, SIBLING, &match, &status);
  }
  */
  MiniTokenName name_match;
  new_node = match_and_add_term_node(cur_node, current_token, names, SIBLING, &name_match, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid Module Declaration: Missing %s\n", desc_token(name_match));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;

  current_token = next_token(current_token, &status);
  if (status != SUCCESS) return status;

  /*
  if (after_token) {
    after_token = next_token(after_token, &status);
    if (status != SUCCESS) return status;
  }
  */
  
  MiniTokenName name = SEMICOLON;
  /*
  if (after_token) {
    new_node = match_and_add_term_node(cur_node, after_token, &name, SIBLING, NULL, &status);
  } else {
    new_node = match_and_add_term_node(cur_node, current_token, &name, SIBLING, NULL, &status);
  }
  */
  new_node = match_and_add_term_node(cur_node, current_token, &name, SIBLING, NULL, &status);

  if (status == NONMATCHING_TOKEN) {
    name = ASSIGN;
    /*
    if (after_token) {
      new_node = match_and_add_term_node(cur_node, after_token, &name, SIBLING, NULL, &status);
    } else {
      new_node = match_and_add_term_node(cur_node, current_token, &name, SIBLING, NULL, &status);
    }
    */
    new_node = match_and_add_term_node(cur_node, current_token, &name, SIBLING, NULL, &status);
    if (status == NONMATCHING_TOKEN) {
      printf("Parse Error: Invalid Module Declaration: Missing %s\n", desc_token(ASSIGN));
      return PARSE_ERROR;
    } else if (status != SUCCESS) return status;

    cur_node = new_node;

    current_token = next_token(current_token, &status);
    if (status != SUCCESS) return status;

    /*
    if (after_token) {
      after_token = next_token(after_token, &status);
      if (status != SUCCESS) return status;
    }
    */

    MiniToken *after_token2 = NULL;
    MiniTokenName names2[] = {LEFT_BRACKET, LEFT_BRACE, -1};
    MiniNonTerm corresp_nonterms[] = {COLLECTION, COLLECTION, -1};
    /*
    if (after_token) {
      new_node = match_and_add_nonterm_node(cur_node, after_token, names2, corresp_nonterms, SIBLING, &match, &status);      
    } else {
      new_node = match_and_add_nonterm_node(cur_node, current_token, names2, corresp_nonterms, SIBLING, &match, &status);      
    }
    */
    new_node = match_and_add_nonterm_node(cur_node, current_token, names2, corresp_nonterms, SIBLING, &name_match, &status);      
    if (status == NONMATCHING_TOKEN) {
      new_node = add_nonterm_node(cur_node, PRIMARY_EXPRESSION, SIBLING, &status);
      if (status != SUCCESS) return status;

      cur_node = new_node; 
     
      /*
      if (after_token) {
        status = expression(cur_node, after_token, &after_token2);
      else {
        status = expression(cur_node, current_token, &after_token2);
      }
      */
      status = primary_expression(cur_node, current_token, &after_token2);
      if (status != VALID_CONSTRUCT) return status;

    } else if (status != SUCCESS) {
      return status;
    } else {

      cur_node = new_node;

      /*
      if (after_token) {
        status = collection(cur_node, after_token, &after_token2);
      else {
        status = collection(cur_node, current_token, &after_token2);
      }
      */
      status = collection(cur_node, current_token, &after_token2);
      if (status != VALID_CONSTRUCT) return status;
    }

    name = SEMICOLON;
    new_node = match_and_add_term_node(cur_node, after_token2, &name, SIBLING, NULL, &status);
    if (status == NONMATCHING_TOKEN) {
      printf("Parse Error: Invalid Module Declaration: Missing %s\n", desc_token(SEMICOLON));
      return PARSE_ERROR;
    } else if (status != SUCCESS) return status;

    cur_node = new_node;

    after_token2 = next_token(after_token2, &status);
    if (status != SUCCESS) return status;

    *token_carrier = after_token2;
    return VALID_CONSTRUCT;

  } else if (status != SUCCESS) {
    return status;
  } else {
    cur_node = new_node;
    /*
    if (after_token) {
      after_token = next_token(after_token, &status);
      if (status != SUCCESS) return status;

      *token_carrier = after_token;
    } else {
      current_token = next_toke(current_token, &status);
      if (status != SUCCESS) return status;

      *token_carrier = current_token;
    }
    */
    current_token = next_token(current_token, &status);
    if (status != SUCCESS) return status;

    *token_carrier = current_token;
    return VALID_CONSTRUCT;
  }
  
  return VALID_CONSTRUCT;
}

/*
// <custom-type> ::= "<" <mini-id> ">"

static MiniStatus custom_type(MiniSyntaxTree *current_node, MiniToken *current_token, MiniToken **token_carrier) {
  if (last_token(current_token)) return LAST_TOKEN;

  MiniStatus status;
  MiniToken *after_token = NULL;
  MiniSyntaxTree *cur_node = current_node;
  MiniSyntaxTree *new_node;
  MiniTokenName names[] = {LESS_THAN, MINI_ID, GREATER_THAN, -1};
  MiniRelation rels[] = {CHILD, SIBLING, SIBLING, -1};
  MiniTokenName non_match;
  new_node = match_and_add_term_node_seq(cur_node, current_token, &after_token, names, rels, &non_match, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid custom type: Missing %s\n", desc_token(non_match));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  *token_carrier = after_token;
  return VALID_CONSTRUCT;
}
*/

// <type-alias> ::= <type-kw> "->" <custom-type> ";"

static MiniStatus type_aliasing(MiniSyntaxTree *current_node, MiniToken *current_token, MiniToken **token_carrier) {
  if (last_token(current_token)) return LAST_TOKEN;

  MiniStatus status;
  //MiniToken *after_token = NULL;
  MiniSyntaxTree *cur_node = current_node;
  MiniSyntaxTree *new_node;
  MiniTokenCat name = TYPE_KW;
  MiniNonTerm corresp_nonterm = TYPE_EXPR;
  new_node = match_cat_and_add_nonterm_node(cur_node, current_token, &name, &corresp_nonterm, CHILD, NULL, &status);
  if (status == NONMATCHING_CATEGORY) {
    printf("Parse Error: Invalid type aliasing: Missing type keyword to alias\n");
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;

  status = type(cur_node, current_token);
  if (status != VALID_CONSTRUCT) return status;

  current_token = next_token(current_token, &status);
  if (status != SUCCESS) return status;

  MiniTokenName name2 = REDIRECT;
  MiniTokenName match;
  new_node = match_and_add_term_node(cur_node, current_token, &name2, SIBLING, &match, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid type aliasing: Missing %s\n", desc_token(REDIRECT));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  current_token = next_token(current_token, &status);
  if (status != SUCCESS) return status;

  cur_node = new_node;

  name2 = CUSTOM_T;
  new_node = match_and_add_term_node(cur_node, current_token, &name2, SIBLING, NULL, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid type aliasing: Missing %s\n", desc_token(CUSTOM_T));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  current_token = next_token(current_token, &status);
  if (status != SUCCESS) return status;
  
  cur_node = new_node;
  
  /*
  MiniToken *after_token2 = NULL;
  status = custom_type(cur_node, after_token, &after_token2);
  if (status != VALID_CONSTRUCT) return status;
  */

  name2 = SEMICOLON;
  new_node = match_and_add_term_node(cur_node, current_token, &name2, SIBLING, &match, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid type aliasing: Missing %s\n", desc_token(SEMICOLON));
  } else if (status != SUCCESS) return status;

  current_token = next_token(current_token, &status);
  if (status != SUCCESS) return status;

  *token_carrier = current_token;
  return VALID_CONSTRUCT; 
}

// <import> ::= ("::" <mini-id> | ("M::" | "C::") <string-literal>) ";"

static MiniStatus import(MiniSyntaxTree *current_node, MiniToken *current_token, MiniToken **token_carrier) {
  if (last_token(current_token)) return LAST_TOKEN;

  MiniStatus status;
  MiniSyntaxTree *cur_node = current_node;
  MiniSyntaxTree *new_node;
  MiniTokenName match;
  MiniTokenName names[] = {IMPORT, M_IMPORT, C_IMPORT, -1};
  new_node = match_and_add_term_node(cur_node, current_token, names, CHILD, &match, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid import statement: Missing %s, %s or %s\n",
        desc_token(IMPORT), desc_token(M_IMPORT), desc_token(C_IMPORT));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  current_token = next_token(current_token, &status);
  if (status != SUCCESS) return status;

  cur_node = new_node;
 
  MiniToken *after_token;
  MiniTokenName non_match;
  MiniTokenName names2[] = {STRING_LITERAL, SEMICOLON, -1};
  MiniRelation rels[] = {CHILD, SIBLING, -1};
  if (match == IMPORT) {
    names2[0] = MINI_ID;
  }
  new_node = match_and_add_term_node_seq(cur_node, current_token, &after_token, names2, rels, &non_match, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid import statement: Missing %s\n", desc_token(non_match));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  cur_node = new_node;

  *token_carrier = after_token;
  return VALID_CONSTRUCT;
}

// <module-seq> ::= (<import> | <typedef> | <module-declaration> | <subprogram>) (<module-sequence> | "")

static MiniStatus module_sequence(MiniSyntaxTree *current_node, MiniToken *current_token, MiniToken **token_carrier) {
  if (last_token(current_token)) return LAST_TOKEN;

  MiniToken *after_token = NULL;
  MiniToken *temp_token = NULL;

  MiniStatus status;
  MiniStatus status2;   
  MiniSyntaxTree *cur_node = current_node;
  MiniSyntaxTree *new_node;
  MiniTokenName match;
  MiniTokenName names[] = {IMPORT, M_IMPORT, C_IMPORT, FUNC, -1};
  MiniNonTerm corresp_nonterms[] = {IMPORTING, IMPORTING, IMPORTING, SUBPROGRAM, -1};
  MiniTokenName names2[] = {REDIRECT, MINI_ID, MINI_CONST_ID, -1};
  MiniNonTerm corresp_nonterms2[] = {TYPE_ALIASING, MODULE_DECLARATION, MODULE_DECLARATION, -1};

  new_node = match_and_add_nonterm_node(cur_node, current_token, names, corresp_nonterms, CHILD, &match, &status);
  if (status == NONMATCHING_TOKEN) {
    temp_token = current_token->next_token;
    new_node = match_and_add_nonterm_node(cur_node, temp_token, names2, corresp_nonterms2, CHILD, &match, &status);
    if (status == NONMATCHING_TOKEN) {
      printf("Parse Error: Invalid module sequence: Should start with\n%s,\n%s,\n%s,\n%s or\ntype keyword\n",
        desc_token(IMPORT), desc_token(M_IMPORT), desc_token(C_IMPORT), desc_token(FUNC));
      return PARSE_ERROR;
    } else if (status != SUCCESS) return status;
    cur_node = new_node;

    if (match == REDIRECT) {
      status = type_aliasing(cur_node, current_token, &after_token);
    } else {
      status = module_declaration(cur_node, current_token, &after_token);
    }
    if (status != VALID_CONSTRUCT) return status;

    if (last_token(after_token)) return LAST_TOKEN;


    status2 = NONMATCHING_TOKEN;
    status = match_terminals(after_token, names, &match);
    if (status == NONMATCHING_TOKEN) {
      if (last_token(after_token->next_token)) {
        *token_carrier = after_token;
        return VALID_CONSTRUCT;  
      }
      status2 = match_terminals(after_token->next_token, names2, &match);
    }

    if (status == SUCCESS || status2 == SUCCESS) {
      new_node = add_nonterm_node(cur_node, MODULE_SEQUENCE, SIBLING, &status);
      if (status != SUCCESS) return status;
      cur_node = new_node;
      return module_sequence(cur_node, after_token, token_carrier);
    } else {
      *token_carrier = after_token;
      return VALID_CONSTRUCT;
    }
    
  } else if (status != SUCCESS) return status;
  cur_node = new_node;

  if (match == IMPORT || match == M_IMPORT || match == C_IMPORT) {
    status = import(cur_node, current_token, &after_token);    
  } else if (match == FUNC) {
    status = subprogram(cur_node, current_token, &after_token);
  } else {
    status = module_declaration(cur_node, current_token, &after_token);
  }
  if (status != VALID_CONSTRUCT) return status;

  if (last_token(after_token)) return LAST_TOKEN;

  status2 = NONMATCHING_TOKEN;
  status = match_terminals(after_token, names, &match);
  if (status == NONMATCHING_TOKEN) {
    if (last_token(after_token->next_token)) {
      *token_carrier = after_token;
      return VALID_CONSTRUCT;  
    }
    status2 = match_terminals(after_token->next_token, names2, &match);
  }

  if (status == SUCCESS || status2 == SUCCESS) {
    new_node = add_nonterm_node(cur_node, MODULE_SEQUENCE, SIBLING, &status);
    if (status != SUCCESS) return status;
    cur_node = new_node;
    return module_sequence(cur_node, after_token, token_carrier);
  } else {
    *token_carrier = after_token;
    return VALID_CONSTRUCT;
  }
}

// <module-part> ::= "}}}" <mini-id> ":" <module-seq> "{{{"

static MiniStatus module_part(MiniSyntaxTree *current_node, MiniToken *current_token, MiniToken **token_carrier) {
  if (last_token(current_token)) return LAST_TOKEN;

  MiniStatus status;
  MiniSyntaxTree *cur_node = current_node;
  MiniSyntaxTree *new_node;
  MiniTokenName non_match;
  MiniTokenName names[] = {MODULE, MINI_ID, COLON, -1};
  MiniRelation rels[] = {CHILD, SIBLING, SIBLING, -1};
  MiniToken *after_token;
  
  new_node = match_and_add_term_node_seq(cur_node, current_token, &after_token, names, rels, &non_match, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid module part specification: Missing %s\n", desc_token(non_match));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;
  cur_node = new_node;

  new_node = add_nonterm_node(cur_node, MODULE_SEQUENCE, SIBLING, &status);
  if (status != SUCCESS) return status;
  cur_node = new_node;

  MiniToken *after_token2 = NULL;
  status = module_sequence(cur_node, after_token, &after_token2);
  if (status != VALID_CONSTRUCT) return status;

  MiniTokenName name = END_MODULE;
  new_node = match_and_add_term_node(cur_node, after_token2, &name, SIBLING, NULL, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid module part specification: Missing %s\n", desc_token(END_MODULE));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  if (last_token(after_token2->next_token)) {
    *token_carrier = NULL;
    return VALID_CONSTRUCT;
  }
  after_token2 = next_token(after_token2, &status);
  if (status != SUCCESS) return status;

  *token_carrier = after_token2;
  return VALID_CONSTRUCT;
}

// <module-file> ::= <module-part>

static MiniStatus module_file(MiniSyntaxTree *current_node, MiniToken *current_token, MiniToken **token_carrier) {
  if (last_token(current_token)) return LAST_TOKEN;
  
  MiniStatus status;
  MiniTokenName name = MODULE;
  MiniNonTerm corresp_nonterm = MODULE_PART;
  MiniSyntaxTree *cur_node = current_node;
  MiniSyntaxTree *new_node;
  
  new_node = match_and_add_nonterm_node(cur_node, current_token, &name, &corresp_nonterm, CHILD, NULL, &status);
  if (status == PARSE_ERROR) {
    printf("Parse Error: Invalid module file: Missing %s\n", desc_token(MODULE)); 
    return PARSE_ERROR;
  }
  if (status != SUCCESS) return status;
  cur_node = new_node;

  return module_part(cur_node, current_token, token_carrier); 
}

// <main-file> ::= "!~>..<~!" (<module-part> | "") <main-part>

static MiniStatus main_file(MiniSyntaxTree *current_node, MiniToken *current_token) {
  if (last_token(current_token)) return LAST_TOKEN;

  MiniStatus status;
  MiniSyntaxTree *cur_node = current_node;
  MiniSyntaxTree *new_node;

  new_node = add_term_node(cur_node, current_token, CHILD, &status);
  if (status != SUCCESS) return status;
  cur_node = new_node;

  current_token = next_token(current_token, &status);
  if (status != SUCCESS) return status;

  MiniTokenName match;
  MiniTokenName names[] = {MAIN, MODULE, -1};
  MiniNonTerm corresp_nonterms[] = {MAIN_PART, MODULE_PART, -1};
  new_node = match_and_add_nonterm_node(cur_node, current_token, names, corresp_nonterms, SIBLING, &match, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid main file specification: Missing %s or %s\n", desc_token(MAIN), desc_token(MODULE));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;
  cur_node = new_node;

  if (match == MAIN) {
    return main_part(cur_node, current_token);
  }

  MiniToken *after_token = NULL;
  status = module_part(cur_node, current_token, &after_token);
  if (status != VALID_CONSTRUCT) return status; 
  
  MiniTokenName name = MAIN;
  MiniNonTerm corresp_nonterm = MAIN_PART;
  new_node = match_and_add_nonterm_node(cur_node, after_token, &name, &corresp_nonterm, SIBLING, NULL, &status);
  if (status == PARSE_ERROR) {
    printf("Parse Error: Invalid main file specification: Missing %s\n", desc_token(MAIN));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;

  return main_part(new_node, after_token);
}

// <source> ::= <module-file> <source> | <module-file> | <main-file>

static MiniStatus source(MiniSyntaxTree *current_node, MiniToken *current_token) {
  if (last_token(current_token)) return LAST_TOKEN;

  MiniStatus status;
  MiniSyntaxTree *cur_node = current_node;
  MiniSyntaxTree *new_node;

  MiniTokenName match;
  MiniTokenName names[] = {MODULE, MAIN_DECLARATION, -1};
  MiniNonTerm corresp_nonterms[] = {MODULE_FILE, MAIN_FILE, -1};
  new_node = match_and_add_nonterm_node(cur_node, current_token, names, corresp_nonterms, CHILD, &match, &status);
  if (status == NONMATCHING_TOKEN) {
    printf("Parse Error: Invalid source specification: Should begin with %s or %s\n", desc_token(MODULE), desc_token(MAIN_DECLARATION));
    return PARSE_ERROR;
  } else if (status != SUCCESS) return status;
  cur_node = new_node;

  if (match == MODULE) {
    MiniToken *after_token = NULL;
    status = module_file(cur_node, current_token, &after_token);
    if (status != VALID_CONSTRUCT) return status;

    if (after_token == NULL) {
      return VALID_CONSTRUCT;
    }

    new_node = add_nonterm_node(cur_node, SOURCE, SIBLING, &status);
    if (status != SUCCESS) return status;

    return source(new_node, after_token);
  } else if (match == MAIN_DECLARATION) {
    return main_file(new_node, current_token);
  }
  return VALID_CONSTRUCT;
}

MiniStatus generate_ast(char *input_file, char *output_file, MiniHeadToken *head_token, MiniSyntaxTree *root, int verbose) {
  if (verbose) {
    printf("Beginning parsing\n");
  }
  size_t len = strlen(input_file);
  if (output_file[0] == '\0') {
    strcpy(output_file, input_file);
    output_file[len - 4] = 'p';
    output_file[len - 3] = 'a';
    output_file[len - 2] = 'r';
    output_file[len - 1] = 's';
  }
  if (verbose) {
    printf("Output file: %s\n", output_file);
  }
  FILE *output_ptr;
  output_ptr = fopen(output_file, "w");
  fprintf(output_ptr, "// Indentation increase = child node to the one above\n// Indentation same = sibling node to the one above\n\n");

  if (head_token == NULL) {
    // TODO: Implement AST generation from .toke file
    printf("AST generation from file not implemented yet\n");
    return PARSE_ERROR;
  }
  MiniToken *current_token = head_token->head;

  MiniStatus status = source(root, current_token);
  
  file_print_syntax_tree(output_ptr, root, 0);
  fclose(output_ptr);
  if (verbose) {
    printf("Parsing complete\n");
  }
  return status;
}


