MODULE Size3;

PROCEDURE P;
  TYPE
    A = ARRAY OF CHAR;
  VAR
    y: LONGINT;
  BEGIN
    y := SIZE(A);
  END P;

BEGIN
  P
END Size3.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)
