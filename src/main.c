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

/* 
  =========================================================================================
  minimal - A command line tool for transpiling Minimal source files to valid C source code
  Written in 2025 by approx-error
  
  To the extent possible under law, the author(s) have dedicated all copyright and related
  and neighboring rights to this software to the public domain worldwide. This software is
  distributed without any warranty.
  
  You should have received a copy of the CC0 Public Domain Dedication along with this
  software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
  =========================================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdint.h>
#include <stdbool.h>
#include "inc/retcodes.h"
#include "inc/options.h"
#include "inc/general.h"
#include "inc/preprocessor.h"
#include "inc/tokens.h"
#include "inc/syntax.h"

#define FILENAME_SIZE 51

static char **alloc_input(int input_file_count, MiniStatus *status) {
  char **input_files = malloc(input_file_count * sizeof(char *));
  if (input_files == NULL) {
    printf("main: Memory Error: Failed to allocate memory for input file array\n");
    free(input_files);
    *status = ALLOCATION_FAIL;
    return NULL;
  }
  for (int i = 0; i < input_file_count; i++) {
    input_files[i] = calloc(FILENAME_SIZE, sizeof(char));
    if (input_files[i] == NULL) {
      printf("main: Memory Error: Failed to allocate memory for input file array element\n");
      for (int j = 0; j < i; j++) {
        free(input_files[j]);
      }
      free(input_files);
      *status = ALLOCATION_FAIL;
      return NULL;
    }
  }
  *status = SUCCESS;
  return input_files;
}

static MiniStatus populate_input(char **input_files, int argc, char *argv[], int start_index) {
  for (int i = start_index; i < argc; i++) {
    char *current = argv[i];
    size_t length = strlen(current);
    if (length > FILENAME_SIZE - 1) {
      printf("main: Error: Maximum filename length is %d\n", FILENAME_SIZE - 1);
      return INVALID_ARG;
    }
    strcpy(input_files[i - start_index], current);
  }
  return SUCCESS;
}

static void free_input(char **input_files, int input_file_count) {
  for (int i = 0; i < input_file_count; i++) {
    free(input_files[i]);
  }
  free(input_files);
}

int main(int argc, char *argv[]) {

  if (argc == 1) {
    return usage(argv[0]);
  }

  bool valid_args = true;
  bool opts_remaining = true;

  char output_file[FILENAME_SIZE] = {'\0'};
  char main_file[FILENAME_SIZE] = {'\0'};
  size_t length;

  int cmd;

  while (valid_args && opts_remaining) {
    int option_index = 0;
    cmd = getopt_long(argc, argv, "vo:", minimal_options, &option_index);

    if (cmd == -1) {
      opts_remaining = false;
      continue;
    }

    switch (cmd) {
      case 0:
        break;
      case 'v':
        verbose_flag = 1;
        break;
      case USAGE:
        return usage(argv[0]);
      case HELP:
        return help(argv[0]);
      case VERSION:
        return version(argv[0]);
      case INFO:
        return version(argv[0]);
      case 'o':
        length = strlen(optarg);
        if (length > FILENAME_SIZE - 1) {
          printf("main: Error: Maximum filename length is %d\n", FILENAME_SIZE - 1);
          valid_args = false;
          break;
        }
        strcpy(output_file, optarg);
        break;
      case '?':
        break;
    }
  }

  if (!valid_args) {
    return INVALID_ARG;
  }

  if (!(optind < argc)) {
    printf("main: Error: No input files specified\n");
    return NO_INPUT_FILE;
  }
  length = strlen(argv[argc - 1]);
  if (length > FILENAME_SIZE - 1) {
    printf("main: Error: Maximum filename length is %d\n", FILENAME_SIZE - 1);
    return INVALID_ARG;
  }
  strcpy(main_file, argv[argc - 1]);

  int input_file_count = argc - optind; 
  MiniStatus status;
  char **input_files = alloc_input(input_file_count, &status); 
  if (status != SUCCESS) return status;

  status = populate_input(input_files, argc, argv, optind); 
  if (status != SUCCESS) {
    free_input(input_files, input_file_count);
    return status;
  }

  char prep_file[FILENAME_SIZE - 1] = {'\0'};
  if (preprocess_flag) {
    strcpy(prep_file, output_file);
  }
  status = preprocess(input_files, input_file_count, main_file, prep_file, verbose_flag);
  if (status != SUCCESS) {
    free_input(input_files, input_file_count);
    return status;
  } 
  free_input(input_files, input_file_count);

  if (preprocess_flag) return SUCCESS;

  MiniHeadToken head_token = {.token_count = 0, .head = NULL};
  char token_file[FILENAME_SIZE - 1] = {'\0'};
  if (tokenize_flag) {
    strcpy(token_file, output_file);
  }
  status = tokenize(prep_file, token_file, &head_token, verbose_flag);
  if (status != SUCCESS || tokenize_flag) {
    free_tokens(&head_token);
    return status;
  }

  /*
  printf("DEBUG: Tokens\n");
  print_tokens(&head_token);
  */

  MiniSyntaxTree syntax_tree_root = {.data.non_terminal = SOURCE, .data_type = NON_TERMINAL, .child = NULL, .sibling = NULL};
  char parse_file[FILENAME_SIZE - 1] = {'\0'};
  if (parse_flag) {
    strcpy(parse_file, output_file);
  }
  status = generate_ast(token_file, parse_file, &head_token, &syntax_tree_root, verbose_flag);
  if (status != VALID_CONSTRUCT) {
    free_tokens(&head_token);
    free_syntax_tree(syntax_tree_root.child);
    return status;
  }

  if (parse_flag) {
    free_tokens(&head_token);
    free_syntax_tree(syntax_tree_root.child);
    return SUCCESS;
  }

  if (verbose_flag) {
    print_syntax_tree(&syntax_tree_root, 0);
  }
  free_tokens(&head_token);

  if (semantic_flag) {
    printf("Semantic analysis and beyond not implemented yet\n");
  }
  free_syntax_tree(syntax_tree_root.child);

  return SUCCESS;
}
