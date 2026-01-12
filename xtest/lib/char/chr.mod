MODULE chr; (* CHR function*)
IMPORT Out;
VAR x, i, c: INTEGER;

BEGIN
       FOR i := 65 TO 90 DO
              Out.Char(CHR(i)); 
       END
       Out.Ln;
        FOR c := 061H TO 07aH DO
              Out.Char(CHR(c)); 
       END
       Out.Ln;
END chr.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
CHECK-NEXT: 0
*)
