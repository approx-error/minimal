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

#include <getopt.h>

#include "inc/options.h"

int verbose_flag = 0;
int preprocess_flag = 0;
int tokenize_flag = 0;
int parse_flag = 0;
int semantic_flag = 0;
int codegen_flag = 1;
int irgen_flag = 0;
int asmgen_flag = 0;
int compile_flag = 0;
int link_flag = 1;

struct option minimal_options[] = {
  // General
  {"usage", no_argument, 0, USAGE},
  {"help", no_argument, 0, HELP},
  {"version", no_argument, 0, VERSION},
  {"info", no_argument, 0, INFO},
  // Flags
  {"verbose", no_argument, &verbose_flag, 1},
  {"pre", no_argument, &preprocess_flag, 1},
  {"lex", no_argument, &tokenize_flag, 1},
  {"syn", no_argument, &parse_flag, 1},
  {"sem", no_argument, &semantic_flag, 1},
  {"cgen", no_argument, &codegen_flag, 1},
  {"ir", no_argument, &irgen_flag, 1},
  {"asm", no_argument, &asmgen_flag, 1},
  {"obj", no_argument, &compile_flag, 1},
  {"exe", no_argument, &link_flag, 1},
  // Options
  {"output", required_argument, 0, 'o'},
  {0, 0, 0, 0}
};
