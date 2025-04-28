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
#include "inc/general.h"
#include "inc/retcodes.h"

MiniStatus usage(char *argv0) {
  printf("Usage: %s [OPTION]\n", argv0);
  printf("   or: %s [OPTION(S)] [SOURCE FILE(S)]\n", argv0);
  printf("   or: %s [OPTION(S)] [SOURCE FILE(S)] --output=<OUTPUT FILE>\n", argv0);
  puts("Transpile SOURCE FILE(S) up to a stage specified by OPTION(S) (default stage is C source code)");
  puts("Optionally define an output file. The default name for the output when C source code is chosen is a.c");
  puts("Run 'minimal --help' for more information on options");
  return SUCCESS;
}

MiniStatus help(char *argv0) {
  printf("Usage: %s [OPTION]\n", argv0);
  printf("   or: %s [OPTION(S)] [SOURCE FILE(S)]\n", argv0);
  printf("   or: %s [OPTION(S)] [SOURCE FILE(S)] --output=<OUTPUT FILE>\n", argv0);
  puts("Transpile SOURCE FILE(S) up to a stage specified by OPTION(S) (default stage is C source code)");
  puts("The source files must have a file extension of '.mini' for them to be recognized by the program");
  puts("An output filename can be chosen. (See below how the default output filename is chosen");
  puts("");
  puts("GENERAL OPTIONS:");
  puts("  --usage      display a short usage text and exit");
  puts("  --help       display this help text and exit");
  puts("  --version    display version information and exit");
  puts("  --info       display info about the program exit");
  puts("OPTIONS:");
  puts("  -o<file>, --output=<file>    choose <file> as the output filename");
  puts("FLAGS:");
  puts("  -v, --verbose    output information about what is being done at each stage");
  puts("  --pre            preprocess only before stopping");
  puts("  --lex            preprocess and perform lexical analysis (tokenization) before stopping");
  puts("  --syn            preprocess and perform lexical and syntactic analysis (parsing) before stopping");
  puts("  --sem            preprocess and perform lexical, syntactic and semantic analysis before stopping");
  puts("  --cgen           produce a single C source file from the input files (This is the default behaviour)");
  puts("  --ir             produce an intermediate representation for the program before stopping");
  puts("  --asm            produce assembler output for the program before stopping");
  puts("  --obj            produce compiled object files for the program before stopping");
  puts("  --exe            produce an executable for the program before stopping");
  puts("");
  puts("The default output file is always of the form <name>.<ext> where <name> is the name of the minimal");
  puts("source code file which contains the main function and <ext> is an extension which depends on the chosen flag:");
  puts("  --pre: <ext> = prep");
  puts("  --lex: <ext> = toke");
  puts("  --syn: <ext> = pars");
  puts("  --sem: <ext> = sema");
  puts(" --cgen: <ext> = c");
  puts("   --ir: <ext> = ir");
  puts("  --asm: <ext> = s");
  puts("  --obj: <ext> = o");
  puts("  --exe: <ext> = out");
  return SUCCESS;
}

static const char *MINIMAL_VERSION = "0.2.0";

MiniStatus version(char *argv0) {
  printf("%s v%s\n", argv0, MINIMAL_VERSION);
  puts("Written in 2025 by approx-error");
  return SUCCESS;
}

MiniStatus info(char *argv0) {
  printf("%s (Minimal) is a simple programming language and transpiler written in C.\n", argv0);
  puts("It is/was developed as a hobby/learning project by approx-error in 2024-2025.");
  puts("As the name implies, the language has a small number of defined constructs that");
  puts("also have very minimal representations. All the defined keywords are short combinations");
  puts("of ASCII-punctuation characters and variable names are restricted to 8 characters");
  puts("The objective of minimal is first and foremost to be a fun learning project that teaches");
  puts("how compilers work. Nonetheless I hope it could also be useful as a scripting language");
  puts("for actually writing simple helper programs etc.");
  puts("Note that the syntax choices of Minimal weren't made with ease of understanding in mind");
  puts("Instead the look of minimal was chosen so that the language wouldn't just look like a C-clone with");
  puts("less features and simply because I wanted there to be a gimmick to the language where it looks like");
  puts("a bunch of nonsense to the untrained eye but manages to communicate a lot with a small amount of symbols");
  puts("to the trained eye. I also wanted my programming language to look cool and for the symbols that make");
  puts("up the language to be used in a consistent way. I personally feel like I have succeeded in that.");
  puts("For more information about the syntax and the design choices that were made, read the documentation.");
  puts("I hope you enjoy using Minimal as much as I enjoyed designing it!");
  puts("approx-error, 2025");
  return SUCCESS;
}
