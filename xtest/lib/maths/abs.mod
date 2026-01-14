MODULE abs; (* ABS function*)
IMPORT Out;
VAR x : INTEGER;

BEGIN
       x := ABS(-3);
       Out.Int(x, 0); Out.Ln;
END abs.

(*
RUN: %comp %s | filecheck %s
CHECK: 3
CHECK-NEXT: 0
*)
