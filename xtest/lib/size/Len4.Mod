MODULE Len4;

IMPORT Out;

PROCEDURE Test* ();
  BEGIN
    Out.Int (LEN ("abc"), 2); Out.Ln;
  END Test;

BEGIN
  Test
END Len4.

(*
RUN: %comp %s | filecheck %s
CHECK: 3
CHECK-NEXT: 0
*)
