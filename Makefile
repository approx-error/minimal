COMPILER := gcc
FLAGS := -Wall -Wextra -Wshadow -Wpointer-arith -Wstrict-prototypes -g # XXX: Remove -g when done!
srcdir := src
objdir := obj
#builddir := build

main_src := main.c
module_src := options.c general.c preprocessor.c tokens.c lexer.c syntax.c parser-utils.c parser.c

exe_name := minimal

src_files := $(patsubst %.c,$(srcdir)/%.c, $(module_src)) $(srcdir)/$(main_src)
obj_files := $(patsubst $(srcdir)/%.c, $(objdir)/%.o, $(src_files))
dep_files := $(patsubst $(objdir)/%.o, $(objdir)/%.d, $(obj_files))
#exe_file := $(builddir)/$(exe_name)

lex_ok_args := --verbose --lex test/lex-ok/lex-ok.mini
lex_ok2_args := --verbose --lex test/lex-ok2/lex-ok2.mini
many_args := --verbose --syn --output=test/many/hello test/many/mod1.mini test/many/mod2.mini test/many/zmain.mini
parse_ok_args := --verbose test/parse-ok/parse-ok.mini
parse_ok2_args := --verbose test/parse-ok2/parse-ok2.mini
extra_tok_args := --verbose test/extra-token/extra-token.mini
wrong_ext_args := --verbose test/wrong-ext/wrong.ext
no_main_args := --verbose test/no-main/no-main.mini

# $(exe_file): $(obj_files)
$(exe_name): $(obj_files)
	@echo Linking object files...
	$(COMPILER) $(FLAGS) $^ -o $@

-include $(dep_files)

$(objdir)/%.o: $(srcdir)/%.c
	@echo Compiling source files...
	$(COMPILER) $(FLAGS) -MMD -c $< -o $@

lexok: $(exe_name)
	@echo Testing lex-ok.mini...
	@echo Expecting success
	./$< $(lex_ok_args)

lexok2: $(exe_name)
	@echo Testing lex-ok2.mini...
	@echo Expecting success
	./$< $(lex_ok2_args)

many: $(exe_name)
	@echo Testing many args...
	@echo Expecting success
	./$< $(many_args)

parseok: $(exe_name)
	@echo Testing parse-ok.mini...
	@echo Expecting success
	./$< $(parse_ok_args)

parseok2: $(exe_name)
	@echo Testing parse-ok2.mini...
	@echo Expecting success
	./$< $(parse_ok2_args)

extratok: $(exe_name)
	@echo Testing extra-token.mini...
	@echo Expecting parse error
	./$< $(extra_tok_args)

wrongext: $(exe_name)
	@echo Testing wrong.ext...
	@echo Expecting preprocess error
	./$< $(wrong_ext_args)

nomain: $(exe_name)
	@echo Testing no-main.mini...
	@echo Expecting success
	./$< $(no_main_args)

clean:
	@echo Cleaning up...
	rm -f $(obj_files) $(dep_files) $(exe_name)
