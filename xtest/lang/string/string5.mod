MODULE string5; (* STRING type *)
IMPORT Out;
VAR   x: STRING;
   i: INTEGER;

BEGIN
   x := 'Hello';
   FOR i := 0 TO LEN(x) - 1 DO
      Out.Char(x[i]); Out.Char(' ');
   END
   Out.Ln;
   RETURN 0;
END string5.

(*
RUN: %comp %s | filecheck %s
CHECK: H e l l o
CHECK-NEXT: 0
*)