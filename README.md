# axcomp

AX compiler

## Lexer

* Integers.
* Identifiers - start with letter, should be UTF-8, can contain '_'.
* Comments: `(* hello sunshine! *)`
* Whitespace: space, tab, newline. UTF-8 encoding for Unicode characters.
* Keywords: `MODULE`, `BEGIN`, `END`, `DIV`, `MOD`, `CONST`, `TYPE`, `VAR`.
* Types: INTEGER (64-bit signed).

## Grammar

Notation:

* `A = B` A devires from B.
* `A | B` either A or B.
* `()` grouping.
* `()*` zero or more times.
* `()+` one or more times (must at least occur once) (`{}` in EBNF).
* `()?` zero or one times (optional) (`[]` in EBNF).  

```ebnf
module = "MODULE" IDENT ";"
          declarations
          "BEGIN"
                statement_seq
          "END" IDENT "."

declarations = ("CONST" (IDENT "=" expr ";")* )?
                ("TYPE" (IDENT "=" type ";")* )?
                ("VAR" (IDENT ":" type ";")* )?

type = INDENT

statement_seq = (statement ";")+

statement = assignment
    | "RETURN" [expr]

assignment = ident ":=" expr

expr = ('+' | '-' )? term ( ('+' | '-' ) term)*

term = factor ( ( '*' | 'DIV' | 'MOD' ) factor)*

factor = IDENT | INTEGER | '(' expr ')'

IDENT = letter (letter | digit | '_')*

INTEGER = digit+
```

## Example program

```pascal
(* Sample program *)
MODULE test;
CONST
    x = 1;
VAR
    z : INTEGER;
BEGIN
    z := 12;
    RETURN (3 * x) + ((2+ (x + 1)) * 4);
END test.
```

## Program options

Program `ax`:

* `-f,--file filename` - compile named file.
* `-m,--main` - the `BEGIN` and `END` part of this module will be executed as the `main()` of the program.

* `-D`string - Debugging options `string` is composed of the following characters:

   `p` - print out parsed tree.
