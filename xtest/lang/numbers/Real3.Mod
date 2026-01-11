(*
RUN: %comp %s | filecheck %s
CHECK: 1
CHECK-NEXT: 2
CHECK-NEXT: -0.025
CHECK-NEXT: 0
*)

MODULE Real3;

IMPORT Out;

PROCEDURE Print(x: REAL);
  BEGIN
    Out.Real(x, 3); Out.Ln;
  END Print;

PROCEDURE f(): REAL;
  BEGIN
    RETURN 1.0;
  END f;

PROCEDURE g(): REAL;
  BEGIN
    RETURN 2.0;
  END g;

PROCEDURE h(): REAL;
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
END Real3.
