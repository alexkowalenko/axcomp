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
* `[]` zero or one times (optional)  

```ebnf
module = "MODULE" IDENT ";"
         declarations
         procedures
         ["BEGIN"
                statement_seq]
         "END" IDENT "."

declarations = ["CONST" (IDENT "=" INTEGER ";")* ]
               ["TYPE" (IDENT "=" type ";")* ]
               ["VAR" (IDENT ":" type ";")* ]

procedures = ( procedureDeclaration ";")*

procedureDeclaration = procedureHeading ";" procedureBody

procedureHeading = "PROCEDURE" IDENT [formalParameters]

formalParameters = "(" [fpection {";" fpSection}] ")" [ ":" IDENT ]

fpSection = ["VAR"] identList ":" type

procedureBody = declarations ["BEGIN" statement_seq] "END" IDENT.

type = INDENT

identList = IDENT ("," IDENT)+

statement_seq = (statement ";")+

statement = assignment
    | procedureCall
    | "RETURN" [expr]

assignment = IDENT ":=" expr

procedureCall = IDENT "(" ")"

expr = ('+' | '-' )? term ( ('+' | '-' ) term)*

term = factor ( ( '*' | 'DIV' | 'MOD' ) factor)*

factor = IDENT | procedureCall | INTEGER | '(' expr ')'

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

    PROCEDURE f;
        VAR yy : INTEGER;
    BEGIN
      RETURN yy + 2;
    END f;

    PROCEDURE g(): INTEGER;
    BEGIN
      RETURN 2;
    END f;

BEGIN
    z := 12;
    f();
    RETURN (3 * f()) + ((2 + (f() + 1)) * 4);
END test.
```
## Notes

Changes to the standard definition:

* Procedures don't have nested procedures.

## Program options

Program `ax`:

* `-f,--file filename` - compile named file.
* `-m,--main` - the `BEGIN` and `END` part of this module will be executed as the `main()` of the program.

* `-D`string - Debugging options `string` is composed of the following characters:

   `p` - print out parsed tree.
