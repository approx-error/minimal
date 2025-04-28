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

#ifndef MINIMAL_PREPROCESSOR_H
#define MINIMAL_PREPROCESSOR_H

extern const char *NO_SEMICOLON_AFTER;
extern const char *MINIMAL_FILE_EXTENSION;
extern const size_t MAX_LINE_LENGTH;

MiniStatus preprocess(char **input_files, int input_file_count, char *main_file, char *output_file, int verbose);

#endif
