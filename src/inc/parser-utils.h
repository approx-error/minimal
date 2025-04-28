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

#ifndef MINIMAL_PARSER_UTILITIES_H
#define MINIMAL_PARSER_UTILITIES_H

#include "retcodes.h"
#include "tokens.h"
#include "syntax.h"

MiniStatus match_terminals(MiniToken *cur_tok, MiniTokenName *targets, MiniTokenName *match);
MiniStatus match_terminal_cats(MiniToken *cur_tok, MiniTokenCat *targets, MiniTokenCat *match);

MiniSyntaxTree *add_term_node(MiniSyntaxTree *cur_node, MiniToken *cur_tok, MiniRelation rel, MiniStatus *status);
MiniSyntaxTree *add_nonterm_node(MiniSyntaxTree *cur_node, MiniNonTerm name, MiniRelation rel, MiniStatus *status);

MiniSyntaxTree *match_and_add_term_node(
  MiniSyntaxTree *cur_node, MiniToken *cur_tok, MiniTokenName *names,
  MiniRelation rel, MiniTokenName *match, MiniStatus *status
);

MiniSyntaxTree *match_and_add_term_node_seq(
  MiniSyntaxTree *cur_node, MiniToken *cur_tok, MiniToken **tok_carrier,
  MiniTokenName *names, MiniRelation *rels, MiniTokenName *non_match, MiniStatus *status 
);

MiniSyntaxTree *match_and_add_nonterm_node(
  MiniSyntaxTree *cur_node, MiniToken *cur_tok, MiniTokenName *names,
  MiniNonTerm *corresp_nonterms, MiniRelation rel, MiniTokenName *match, MiniStatus *status  
);

MiniSyntaxTree *match_cat_and_add_nonterm_node(
  MiniSyntaxTree *current_node, MiniToken *current_tok, MiniTokenCat *cats,
  MiniNonTerm *corresp_nonterms, MiniRelation rel, MiniTokenCat *match, MiniStatus *status  
);
#endif
