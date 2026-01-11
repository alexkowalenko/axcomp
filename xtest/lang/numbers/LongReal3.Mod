(*
RUN: %comp %s | filecheck %s
CHECK: 1
CHECK-NEXT: 2
CHECK-NEXT: -0.025
CHECK-NEXT: 0
*)

MODULE LongReal3;

IMPORT Out;

PROCEDURE Print(x: LONGREAL);
  BEGIN
    Out.LongReal(x, 3); Out.Ln;
  END Print;

PROCEDURE f(): LONGREAL;
  BEGIN
    RETURN 1.0;
  END f;

PROCEDURE g(): LONGREAL;
  BEGIN
    RETURN 2.0;
  END g;

PROCEDURE h(): LONGREAL;
  BEGIN
    RETURN -0.025;
  END h;

PROCEDURE Test*();
  BEGIN
    Print(f());
    Print(g());
    Print(h());
  END Test;

BEGIN
  Test
END LongReal3.
