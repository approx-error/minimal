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
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "inc/retcodes.h"
#include "inc/preprocessor.h"

const char *NO_SEMICOLON_AFTER = ":?#@$";
const char *MINIMAL_FILE_EXTENSION = "mini";
const size_t MAX_LINE_LENGTH = 100;

static void trim_string(char *string) {
  size_t start = 0;
  size_t end = strlen(string) - 1;

  while (isspace(string[start])) {
    start++;
  }

  while (end > start && isspace(string[end])) {
    end--;
  }

  if (start > 0 || end < (strlen(string) - 1)) {
    memmove(string, string + start, end - start + 1);
    string[end - start + 1] = '\0';
  }
}

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

static bool should_add_semicolon(char *string) {
  char last_symbol = string[strlen(string) - 1];
  size_t no_semicolon_count = strlen(NO_SEMICOLON_AFTER);
  for (size_t i = 0; i < no_semicolon_count; i++) {
    if (last_symbol == NO_SEMICOLON_AFTER[i]) {
      return false;
    }
  }
  if (strcmp(string, "<<<") == 0 || strcmp(string, "{{{") == 0 || strcmp(string, "!~>..<~!") == 0) {
    return false;
  }
  return true;
}

MiniStatus preprocess(char **input_files, int input_file_count, char *main_file, char *output_file, int verbose) {
  if (verbose) {
    printf("Beginning preprocessing\n");
  }
  size_t filename_len = strlen(main_file);
  if (output_file[0] == '\0') {
    strcpy(output_file, main_file);  
    output_file[filename_len - 4] = 'p';
    output_file[filename_len - 3] = 'r';
    output_file[filename_len - 2] = 'e';
    output_file[filename_len - 1] = 'p';
  }
  if (verbose) {
    printf("Output file: %s\n", output_file);
  }
  
  for (int i = 0; i < input_file_count; i++) {
    char *current_file = input_files[i];
    size_t current_len = strlen(current_file);
    for (int j = 0; j < 4; j++) {
      if (MINIMAL_FILE_EXTENSION[j] != current_file[current_len - (4 - j)]) {
        printf("preprocess: File Error: Source file %s doesn't have correct extension '.mini'\n", current_file);
        return INVALID_ARG;
      }
    }
  }

  FILE *output_ptr;
  output_ptr = fopen(output_file, "w");

  FILE *input_ptr;
  for (int i = 0; i < input_file_count; i++) {
    
    char *current_file = input_files[i];
    input_ptr = fopen(current_file, "r");
    if (input_ptr == NULL) {
      printf("preprocess: File Error: Source file %s couldn't be found!\n", current_file);
      fclose(output_ptr);
      return FILE_NOT_FOUND;
    }
    if (verbose) {
      printf("Current file: %s\n", current_file);
    }

    char line_buffer[MAX_LINE_LENGTH + 2]; // Enough space for MAX_LINE_LENGTH characters, a newline character, and a null terminator
    int line_count = 0;
    size_t line_length;
    int comment_counter = 0;
    while (fgets(line_buffer, MAX_LINE_LENGTH + 2, input_ptr) != NULL) {
      line_length = strcspn(line_buffer, "\n");
      //printf("Line: %d, Length: %d\n", line_count + 1, line_length);
      if (line_length > MAX_LINE_LENGTH) {
          printf("preprocess: Syntax error: Line %d too long! Maximum is %ld characters\n", line_count + 1, MAX_LINE_LENGTH);
          fclose(input_ptr);
          fclose(output_ptr);
          return LINE_TOO_LONG; 
      }
      line_buffer[line_length] = '\0';
      trim_string(line_buffer);

      if (is_comment(line_buffer)) {
          fprintf(output_ptr, "%s\n", line_buffer);
          comment_counter++;
      } else {
        char *token;
        char *saveptr = line_buffer;
        while ((token = strtok_r(saveptr, ";", &saveptr))) {
          trim_string(token);
          //printf("Current token: %s\n", token);
          bool semicolon = should_add_semicolon(token);
          if (semicolon) { 
            if (strlen(token) == MAX_LINE_LENGTH) {
              printf("preprocess: Syntax error: Line %d too long to add a delimiter! On lines that need a delimiter, maximum is %ld characters + 1 ';'\n", line_count + 1, MAX_LINE_LENGTH - 1);
              return CANT_ADD_DELIMITER; 
            }
            fprintf(output_ptr, "%s;\n", token);
          } else {
            fprintf(output_ptr, "%s\n", token);
          }
        }
      }
      line_count++;
    }

    if (line_count == 0) {
      printf("preprocess: File Error: Source file %s was empty!\n", current_file);
      fclose(input_ptr);
      fclose(output_ptr);
      return FILE_EMPTY;
    }

    fclose(input_ptr);
  } 
  fclose(output_ptr);
  if (verbose) {
    printf("Preprocessing complete\n");
  }
  return SUCCESS;
}
