# axcomp

AX compiler

## Lexer

* Integers.
* Identifiers - start with letter, should be UTF-8, can contain '_'.
* Comments: `(* hello sunshine! *)`
* Whitespace: space, tab, newline. UTF-8 encoding for Unicode characters.
* Keywords: `MODULE`, `BEGIN`, `END`.

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
          "BEGIN"
                statement_seq
          "END" IDENT "."
          
statement_seq -> (expr ";")+

expr -> ('+' | '-' )? term ( ('+' | '-' ) term)*

term -> factor ( ( '*' | 'DIV' | 'MOD' ) factor)*

factor -> INTEGER | '(' expr ')'

IDENT -> letter (letter | digit | '_')*

INTEGER -> digit+
```

## Example program

```pascal
(* Sample program *)
MODULE test;
BEGIN
    12;
    24;
END test.
```
