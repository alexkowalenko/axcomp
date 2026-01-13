MODULE Div3;

IMPORT Out;

PROCEDURE Div (i: INTEGER);
  BEGIN
    Out.Int (i DIV 2, 3); Out.Ln;
  END Div;

PROCEDURE Test* ();
  BEGIN
    Div (2);
    Div (-2);
  END Test;

BEGIN
  Test
END Div3.

(*
RUN: %comp %s | filecheck %s
CHECK: 1
CHECK-NEXT: -1
*)
