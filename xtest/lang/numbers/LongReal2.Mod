(*
RUN: %comp %s | filecheck %s
CHECK: 1
CHECK-NEXT: 2
CHECK-NEXT: 0.025
CHECK-NEXT: 0
*)

MODULE LongReal2;

IMPORT Out;

PROCEDURE Print(x: LONGREAL);
  BEGIN
    Out.LongReal(x, 3); Out.Ln;
  END Print;

PROCEDURE Print2(x: LONGREAL);
  BEGIN
    Print(x);
  END Print2;

PROCEDURE Test*();
  BEGIN
    Print2(1.0);
    Print2(2.0);
    Print2(-0.025);
  END Test;

BEGIN
  Test
END LongReal2.
