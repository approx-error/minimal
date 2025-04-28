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

#ifndef MINIMAL_RETURN_CODES_H
#define MINIMAL_RETURN_CODES_H

typedef enum return_codes {
  SUCCESS = 0,
  ALLOCATION_FAIL,
  REALLOCATION_FAIL,
  INVALID_ARG,
  NO_INPUT_FILE,
  TOO_MANY_ARGS,
  FILE_NOT_FOUND,
  FILE_EMPTY,
  LINE_TOO_LONG,
  CANT_ADD_DELIMITER,
  INVALID_SYNTAX,
  PARSE_ERROR,
  NONMATCHING_TOKEN,
  NONMATCHING_CATEGORY,
  LAST_TOKEN,
  VALID_CONSTRUCT,
  INVALID_CONSTRUCT
} MiniStatus;

#endif
