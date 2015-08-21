BUILD LLVM/CLANG:
----------------
# Build clang as explained here 'http://clang.llvm.org/get_started.html'

$ ./build.sh
$ ./clang-lexer
struct 'struct'	 [StartOfLine]	Loc=<simple.c:1:1>
identifier 'dummy'	 [LeadingSpace]	Loc=<simple.c:1:8>
l_brace '{'	 [LeadingSpace]	Loc=<simple.c:1:14>
int 'int'	 [StartOfLine] [LeadingSpace]	Loc=<simple.c:2:3>
identifier 'x'	 [LeadingSpace]	Loc=<simple.c:2:7>
semi ';'		Loc=<simple.c:2:8>
char 'char'	 [StartOfLine] [LeadingSpace]	Loc=<simple.c:3:3>
identifier 'y'	 [LeadingSpace]	Loc=<simple.c:3:8>
semi ';'		Loc=<simple.c:3:9>
r_brace '}'	 [StartOfLine]	Loc=<simple.c:4:1>
semi ';'		Loc=<simple.c:4:2>
float 'float'	 [StartOfLine]	Loc=<simple.c:6:1>
identifier 'func'	 [LeadingSpace]	Loc=<simple.c:6:7>
l_paren '('		Loc=<simple.c:6:11>
int 'int'		Loc=<simple.c:6:12>
identifier 'a'	 [LeadingSpace]	Loc=<simple.c:6:16>
r_paren ')'		Loc=<simple.c:6:17>
l_brace '{'	 [StartOfLine]	Loc=<simple.c:7:1>
float 'float'	 [StartOfLine] [LeadingSpace]	Loc=<simple.c:8:3>
identifier 'b'	 [LeadingSpace]	Loc=<simple.c:8:9>
equal '='	 [LeadingSpace]	Loc=<simple.c:8:11>
identifier 'a'	 [LeadingSpace]	Loc=<simple.c:8:13>
plus '+'	 [LeadingSpace]	Loc=<simple.c:8:15>
numeric_constant '10'	 [LeadingSpace]	Loc=<simple.c:8:17>
semi ';'		Loc=<simple.c:8:19>
return 'return'	 [StartOfLine] [LeadingSpace]	Loc=<simple.c:9:3>
identifier 'b'	 [LeadingSpace]	Loc=<simple.c:9:10>
semi ';'		Loc=<simple.c:9:11>
r_brace '}'	 [StartOfLine]	Loc=<simple.c:10:1>
int 'int'	 [StartOfLine]	Loc=<simple.c:12:1>
identifier 'main'	 [LeadingSpace]	Loc=<simple.c:12:5>
l_paren '('		Loc=<simple.c:12:9>
r_paren ')'		Loc=<simple.c:12:10>
l_brace '{'	 [StartOfLine]	Loc=<simple.c:13:1>
const 'const'	 [StartOfLine] [LeadingSpace]	Loc=<simple.c:14:5>
int 'int'	 [LeadingSpace]	Loc=<simple.c:14:11>
identifier 'k'	 [LeadingSpace]	Loc=<simple.c:14:15>
equal '='	 [LeadingSpace]	Loc=<simple.c:14:17>
numeric_constant '5'	 [LeadingSpace]	Loc=<simple.c:14:19>
semi ';'		Loc=<simple.c:14:20>
int 'int'	 [StartOfLine] [LeadingSpace]	Loc=<simple.c:15:4>
star '*'	 [LeadingSpace]	Loc=<simple.c:15:8>
identifier 'kptr'		Loc=<simple.c:15:9>
equal '='	 [LeadingSpace]	Loc=<simple.c:15:14>
amp '&'	 [LeadingSpace]	Loc=<simple.c:15:16>
identifier 'k'		Loc=<simple.c:15:17>
semi ';'		Loc=<simple.c:15:18>
star '*'	 [StartOfLine] [LeadingSpace]	Loc=<simple.c:16:4>
identifier 'kptr'		Loc=<simple.c:16:5>
equal '='	 [LeadingSpace]	Loc=<simple.c:16:10>
numeric_constant '7'	 [LeadingSpace]	Loc=<simple.c:16:12>
semi ';'		Loc=<simple.c:16:13>
return 'return'	 [StartOfLine] [LeadingSpace]	Loc=<simple.c:17:4>
numeric_constant '0'	 [LeadingSpace]	Loc=<simple.c:17:11>
semi ';'		Loc=<simple.c:17:12>
r_brace '}'	 [StartOfLine]	Loc=<simple.c:18:1>
eof ''		Loc=<simple.c:18:2>
