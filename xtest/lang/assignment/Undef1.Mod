(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)

MODULE Undef1;

PROCEDURE P*();
  VAR
    x, y: INTEGER;
  BEGIN
    x := y;
  END P;

BEGIN
  P;
END Undef1.
