/**


\page doc_script_bnf Script language grammar

This is language grammar in Backus-Naur Form (BNF).

The following convention is used:

<pre>
 ( )  - used for grouping
 { }  - 0 or more repetitions
 [ ]  - optional
  |   - or
 ' '  - token
</pre>

<pre>
SCRIPT        ::= {IMPORT | ENUM | TYPEDEF | CLASS | MIXIN | INTERFACE | FUNCDEF | VIRTPROP | VAR | FUNC | NAMESPACE | ';'}
CLASS         ::= {'shared' | 'abstract' | 'final' | 'external'} 'class' IDENTIFIER (';' | ([':' IDENTIFIER {',' IDENTIFIER}] '{' {VIRTPROP | FUNC | VAR | FUNCDEF} '}'))
TYPEDEF       ::= 'typedef' PRIMTYPE IDENTIFIER ';'
NAMESPACE     ::= 'namespace' IDENTIFIER '{' SCRIPT '}'
FUNC          ::= {'shared' | 'external'} ['private' | 'protected'] [((TYPE ['&']) | '~')] IDENTIFIER PARAMLIST ['const'] FUNCATTR (';' | STATBLOCK)
INTERFACE     ::= {'external' | 'shared'} 'interface' IDENTIFIER (';' | ([':' IDENTIFIER {',' IDENTIFIER}] '{' {VIRTPROP | INTFMTHD} '}'))
VAR           ::= ['private'|'protected'] TYPE IDENTIFIER [( '=' (INITLIST | EXPR)) | ARGLIST] {',' IDENTIFIER [( '=' (INITLIST | EXPR)) | ARGLIST]} ';'
IMPORT        ::= 'import' TYPE ['&'] IDENTIFIER PARAMLIST FUNCATTR 'from' STRING ';'
ENUM          ::= {'shared' | 'external'} 'enum' IDENTIFIER (';' | ('{' IDENTIFIER ['=' EXPR] {',' IDENTIFIER ['=' EXPR]} '}'))
FUNCDEF       ::= {'external' | 'shared'} 'funcdef' TYPE ['&'] IDENTIFIER PARAMLIST ';'
VIRTPROP      ::= ['private' | 'protected'] TYPE ['&'] IDENTIFIER '{' {('get' | 'set') ['const'] FUNCATTR (STATBLOCK | ';')} '}'
MIXIN         ::= 'mixin' CLASS
INTFMTHD      ::= TYPE ['&'] IDENTIFIER PARAMLIST ['const'] ';'
STATBLOCK     ::= '{' {VAR | STATEMENT} '}'
PARAMLIST     ::= '(' ['void' | (TYPE TYPEMOD [IDENTIFIER] ['=' EXPR] {',' TYPE TYPEMOD [IDENTIFIER] ['=' EXPR]})] ')'
TYPEMOD       ::= ['&' ['in' | 'out' | 'inout']]
TYPE          ::= ['const'] SCOPE DATATYPE ['<' TYPE {',' TYPE} '>'] { ('[' ']') | ('@' ['const']) }
INITLIST      ::= '{' [ASSIGN | INITLIST] {',' [ASSIGN | INITLIST]} '}'
SCOPE         ::= ['::'] {IDENTIFIER '::'} [IDENTIFIER ['<' TYPE {',' TYPE} '>'] '::']
DATATYPE      ::= (IDENTIFIER | PRIMTYPE | '?' | 'auto')
PRIMTYPE      ::= 'void' | 'int' | 'int8' | 'int16' | 'int32' | 'int64' | 'uint' | 'uint8' | 'uint16' | 'uint32' | 'uint64' | 'float' | 'double' | 'bool'
FUNCATTR      ::= {'override' | 'final' | 'explicit' | 'property'}
STATEMENT     ::= (IF | FOR | WHILE | RETURN | STATBLOCK | BREAK | CONTINUE | DOWHILE | SWITCH | EXPRSTAT | TRY)
SWITCH        ::= 'switch' '(' ASSIGN ')' '{' {CASE} '}'
BREAK         ::= 'break' ';'
FOR           ::= 'for' '(' (VAR | EXPRSTAT) EXPRSTAT [ASSIGN {',' ASSIGN}] ')' STATEMENT
WHILE         ::= 'while' '(' ASSIGN ')' STATEMENT
DOWHILE       ::= 'do' STATEMENT 'while' '(' ASSIGN ')' ';'
IF            ::= 'if' '(' ASSIGN ')' STATEMENT ['else' STATEMENT]
CONTINUE      ::= 'continue' ';'
EXPRSTAT      ::= [ASSIGN] ';'
TRY           ::= 'try' STATBLOCK 'catch' STATBLOCK
RETURN        ::= 'return' [ASSIGN] ';'
CASE          ::= (('case' EXPR) | 'default') ':' {STATEMENT}
EXPR          ::= EXPRTERM {EXPROP EXPRTERM}
EXPRTERM      ::= ([TYPE '='] INITLIST) | ({EXPRPREOP} EXPRVALUE {EXPRPOSTOP})
EXPRVALUE     ::= 'void' | CONSTRUCTCALL | FUNCCALL | VARACCESS | CAST | LITERAL | '(' ASSIGN ')' | LAMBDA
CONSTRUCTCALL ::= TYPE ARGLIST
EXPRPREOP     ::= '-' | '+' | '!' | '++' | '--' | '~' | '@'
EXPRPOSTOP    ::= ('.' (FUNCCALL | IDENTIFIER)) | ('[' [IDENTIFIER ':'] ASSIGN {',' [IDENTIFIER ':' ASSIGN} ']') | ARGLIST | '++' | '--'
CAST          ::= 'cast' '<' TYPE '>' '(' ASSIGN ')'
LAMBDA        ::= 'function' '(' [[TYPE TYPEMOD] IDENTIFIER {',' [TYPE TYPEMOD] IDENTIFIER}] ')' STATBLOCK
LITERAL       ::= NUMBER | STRING | BITS | 'true' | 'false' | 'null'
FUNCCALL      ::= SCOPE IDENTIFIER ARGLIST
VARACCESS     ::= SCOPE IDENTIFIER
ARGLIST       ::= '(' [IDENTIFIER ':'] ASSIGN {',' [IDENTIFIER ':'] ASSIGN} ')'
ASSIGN        ::= CONDITION [ ASSIGNOP ASSIGN ]
CONDITION     ::= EXPR ['?' ASSIGN ':' ASSIGN]
EXPROP        ::= MATHOP | COMPOP | LOGICOP | BITOP
BITOP         ::= '&' | '|' | '^' | '<<' | '>>' | '>>>'
MATHOP        ::= '+' | '-' | '*' | '/' | '%' | '**'
COMPOP        ::= '==' | '!=' | '<' | '<=' | '>' | '>=' | 'is' | '!is'
LOGICOP       ::= '&&' | '||' | '^^' | 'and' | 'or' | 'xor'
ASSIGNOP      ::= '=' | '+=' | '-=' | '*=' | '/=' | '|=' | '&=' | '^=' | '%=' | '**=' | '<<=' | '>>=' | '>>>='
IDENTIFIER    ::= single token:  starts with letter or _, can include any letter and digit, same as in C++
NUMBER        ::= single token:  includes integers and real numbers, same as C++
STRING        ::= single token:  single quoted ', double quoted ", or heredoc multi-line string """
BITS          ::= single token:  binary 0b or 0B, octal 0o or 0O, decimal 0d or 0D, hexadecimal 0x or 0X
</pre>




*/
