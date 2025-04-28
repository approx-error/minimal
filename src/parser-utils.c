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
#include <stdint.h>

#include "inc/retcodes.h"
#include "inc/tokens.h"
#include "inc/syntax.h"

static MiniTokenName match_terminal(MiniToken *current_tok, MiniTokenName target) {
  if (current_tok->name == target) {
    return target;
  } else {
    return TOKEN_UNDETERMINED;
  }
}

static MiniTokenCat match_terminal_category(MiniToken *current_tok, MiniTokenCat target) {
  if (current_tok->category == target) {
    return target;
  } else {
    return CATEGORY_UNDETERMINED;
  }
}

MiniStatus match_terminals(MiniToken *current_tok, MiniTokenName *targets, MiniTokenName *match) {
  MiniTokenName result;
  if (match == NULL) {
    result = match_terminal(current_tok, *targets);
    if (result != TOKEN_UNDETERMINED) {
      return SUCCESS;
    }
    return NONMATCHING_TOKEN;
  }

  MiniTokenName *cur_name = targets;
  while (*cur_name != -1) {
    result = match_terminal(current_tok, *cur_name);
    if (result != TOKEN_UNDETERMINED) {
      *match = result;
      return SUCCESS;
    }
    cur_name++;
  }
  *match = *cur_name;
  return NONMATCHING_TOKEN;
}

MiniStatus match_terminal_cats(MiniToken *current_tok, MiniTokenCat *targets, MiniTokenCat *match) {
  MiniTokenCat result;
  if (match == NULL) {
    result = match_terminal_category(current_tok, *targets);
    if (result != CATEGORY_UNDETERMINED) {
      return SUCCESS;
    }
    return NONMATCHING_CATEGORY;
  }

  MiniTokenCat *cur_name = targets;
  while (*cur_name != -1) {
    result = match_terminal_category(current_tok, *cur_name);
    if (result != CATEGORY_UNDETERMINED) {
      *match = result;
      return SUCCESS;
    }
    cur_name++;
  }
  *match = *cur_name;
  return NONMATCHING_CATEGORY;
}

MiniSyntaxTree *add_term_node(MiniSyntaxTree *current_node, MiniToken *current_token, MiniRelation rel, MiniStatus *exit_status) {
  MiniStatus status;

  MiniSyntaxTree *new_node = alloc_syntax_tree(&status);
  if (status != SUCCESS) {
    *exit_status = status;
    return NULL;
  }

  MiniGramCons *construct = alloc_gram_construct(&status);
  if (status != SUCCESS) {
    *exit_status = status;
    return NULL;
  }
  construct->token = *current_token;

  status = init_syntax_tree(new_node, construct, TOKEN);
  if (status != SUCCESS) {
    *exit_status = status;
    return NULL;
  }

  add_node(current_node, new_node, rel);
  *exit_status = SUCCESS;
  return new_node;
} 

MiniSyntaxTree *add_nonterm_node(MiniSyntaxTree *current_node, MiniNonTerm name, MiniRelation rel, MiniStatus *exit_status) {
  MiniStatus status;

  MiniSyntaxTree *new_node = alloc_syntax_tree(&status);
  if (status != SUCCESS) {
    *exit_status = status;
    return NULL;
  }

  MiniGramCons *construct = alloc_gram_construct(&status);
  if (status != SUCCESS) {
    *exit_status = status;
    return NULL;
  }
  construct->non_terminal = name;

  status = init_syntax_tree(new_node, construct, NON_TERMINAL);
  if (status != SUCCESS) {
    *exit_status = status;
    return NULL;
  }

  add_node(current_node, new_node, rel);
  *exit_status = SUCCESS;
  return new_node;
}

MiniSyntaxTree *match_and_add_term_node(MiniSyntaxTree *current_node, MiniToken *current_token, MiniTokenName *names, MiniRelation rel, MiniTokenName *match, MiniStatus *exit_status) {
  MiniStatus status;
  MiniTokenName result;
  MiniSyntaxTree *new_node;

  if (match == NULL) {
    result = match_terminal(current_token, *names);
    if (result != TOKEN_UNDETERMINED) {
      new_node = add_term_node(current_node, current_token, rel, &status);
      if (status != SUCCESS) {
        *exit_status = status;
        return NULL;
      }
      *exit_status = SUCCESS;
      return new_node;
    } else {
      *exit_status = NONMATCHING_TOKEN;
      return NULL;
    }
  } 

  MiniTokenName *cur_name = names;
  while (*cur_name != -1) {
    result = match_terminal(current_token, *cur_name);
    if (result != TOKEN_UNDETERMINED) {
      new_node = add_term_node(current_node, current_token, rel, &status);
      if (status != SUCCESS) {
        *exit_status = status;
        *match = *cur_name;
        return NULL;
      }
      *exit_status = SUCCESS;
      *match = result;
      return new_node;
    }
    cur_name++;
  }
  *exit_status = NONMATCHING_TOKEN;
  *match = *cur_name;
  return NULL;
} 

MiniSyntaxTree *match_and_add_term_node_seq(MiniSyntaxTree *current_node, MiniToken *current_tok, MiniToken **tok_carrier, MiniTokenName *names, MiniRelation *rels, MiniTokenName *non_match, MiniStatus *exit_status) {
  MiniStatus status;
  MiniTokenName result;
  MiniSyntaxTree *new_node;
  
  MiniSyntaxTree *cur_node = current_node;
  MiniTokenName *cur_name = names;
  MiniToken *cur_token = current_tok;
  MiniRelation *cur_rel = rels;
  while (*cur_name != -1) {
    result = match_terminal(cur_token, *cur_name);
    if (result != TOKEN_UNDETERMINED) {
      new_node = add_term_node(cur_node, cur_token, *cur_rel, &status);
      if (status != SUCCESS) {
        *exit_status = status;
        *non_match = *cur_name;
        *tok_carrier = cur_token;
        return NULL;
      }
    } else {
      *exit_status = NONMATCHING_TOKEN;
      *non_match = *cur_name;
      *tok_carrier = cur_token;
      return NULL;
    }
    cur_node = new_node;
    cur_name++;
    cur_token = next_token(cur_token, &status);
    if (status != SUCCESS) {
      *exit_status = status;
      *non_match = *cur_name;
      *tok_carrier = cur_token;
      return NULL;
    }
    cur_rel++;
  }
  *exit_status = SUCCESS;
  *non_match = *cur_name;
  *tok_carrier = cur_token;
  return cur_node;
}

MiniSyntaxTree *match_and_add_nonterm_node(MiniSyntaxTree *current_node, MiniToken *current_tok, MiniTokenName *names, MiniNonTerm *corresp_nonterms, MiniRelation rel, MiniTokenName *match, MiniStatus *exit_status) {
  MiniStatus status;
  MiniTokenName result;
  MiniSyntaxTree *new_node;

  if (match == NULL) {
    result = match_terminal(current_tok, *names);
    if (result != TOKEN_UNDETERMINED) {
      new_node = add_nonterm_node(current_node, *corresp_nonterms, rel, &status);
      if (status != SUCCESS) {
        *exit_status = status;
        return NULL;
      }
      *exit_status = SUCCESS;
      return new_node;
    } else {
      *exit_status = NONMATCHING_TOKEN;
      return NULL;
    }
  } 

  MiniTokenName *cur_name = names;
  MiniNonTerm *cur_correspondence = corresp_nonterms;
  while (*cur_name != -1) {
    result = match_terminal(current_tok, *cur_name);
    if (result != TOKEN_UNDETERMINED) {
      new_node = add_nonterm_node(current_node, *cur_correspondence, rel, &status);
      if (status != SUCCESS) {
        *exit_status = status;
        *match = *cur_name;
        return NULL;
      }
      *exit_status = SUCCESS;
      *match = *cur_name;
      return new_node;
    }
    cur_name++;
    cur_correspondence++;
  }
  *exit_status = NONMATCHING_TOKEN;
  *match = *cur_name;
  return NULL;
}

MiniSyntaxTree *match_cat_and_add_nonterm_node(MiniSyntaxTree *current_node, MiniToken *current_tok, MiniTokenCat *cats, MiniNonTerm *corresp_nonterms, MiniRelation rel, MiniTokenCat *match, MiniStatus *exit_status) {
  MiniStatus status;
  MiniTokenCat result;
  MiniSyntaxTree *new_node;

  if (match == NULL) {
    result = match_terminal_category(current_tok, *cats);
    if (result != CATEGORY_UNDETERMINED) {
      new_node = add_nonterm_node(current_node, *corresp_nonterms, rel, &status);
      if (status != SUCCESS) {
        *exit_status = status;
        return NULL;
      }
      *exit_status = SUCCESS;
      return new_node;
    } else {
      *exit_status = NONMATCHING_CATEGORY;
      return NULL;
    }
  } 

  MiniTokenCat *cur_cat = cats;
  MiniNonTerm *cur_correspondence = corresp_nonterms;
  while (*cur_cat != -1) {
    result = match_terminal_category(current_tok, *cur_cat);
    if (result != CATEGORY_UNDETERMINED) {
      new_node = add_nonterm_node(current_node, *cur_correspondence, rel, &status);
      if (status != SUCCESS) {
        *exit_status = status;
        *match = *cur_cat;
        return NULL;
      }
      *exit_status = SUCCESS;
      *match = *cur_cat;
      return new_node;
    }
    cur_cat++;
    cur_correspondence++;
  }
  *exit_status = NONMATCHING_CATEGORY;
  *match = *cur_cat;
  return NULL;
}
