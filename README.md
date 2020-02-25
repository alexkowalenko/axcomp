# axcomp

AX compiler

## Lexer

* Integers.
* Identifiers - start with letter, should be UTF-8, can contain '_'.
* Comments: `(* hello sunshine! *)`
* Whitespace: space, tab, newline. UTF-8 encoding for Unicode characters.
* Keywords: `MODULE`, `BEGIN`, `END`, `DIV`, `MOD`, `CONST`, `TYPE`, `VAR`.

## Grammar

Notation:

* `A -> B` A devires from B.
* `A | B` either A or B.
* `()` grouping.
* `()*` zero or more times.
* `()+` one or more times (must at least occur once) (`{}` in EBNF).
* `()?` zero or one times (optional) (`[]` in EBNF).  

```ebnf
module -> "MODULE" IDENT ";"
          declarations
          "BEGIN"
                statement_seq
          "END" IDENT "."

declarations -> ("CONST" (IDENT "=" expr ";")* )?
                ("TYPE" (IDENT "=" type ";")* )?
                ("VAR" (IDENT ":" type ";")* )?

type = INDENT

statement_seq -> (expr ";")+

expr -> ('+' | '-' )? term ( ('+' | '-' ) term)*

term -> factor ( ( '*' | 'DIV' | 'MOD' ) factor)*

factor -> IDENT | INTEGER | '(' expr ')'

IDENT -> letter (letter | digit | '_')*

INTEGER -> digit+
```

## Example program

```pascal
(* Sample program *)
MODULE test;
CONST
    x = 1;
BEGIN
    12;
    (3 * x) + ((2+ (x + 1)) * 4);
END test.
```
