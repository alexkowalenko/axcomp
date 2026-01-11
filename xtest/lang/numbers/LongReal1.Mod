(*
RUN: %comp %s | filecheck %s
CHECK: 1
CHECK-NEXT: 2
CHECK-NEXT: -0.025
CHECK-NEXT: 0
*)

MODULE LongReal1;

IMPORT Out;

PROCEDURE Print(x: LONGREAL);
  BEGIN
    Out.LongReal(x, 3); Out.Ln;
  END Print;

PROCEDURE Test*();
  BEGIN
    Print(1.0);
    Print(2.0);
    Print(-0.025);
  END Test;

BEGIN
  Test
END LongReal1.
