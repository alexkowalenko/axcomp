MODULE Size1;

PROCEDURE P;
  VAR
    x: INTEGER;
    y: LONGINT;
  BEGIN
    y := SIZE(x);
  END P;

BEGIN
  P
END Size1.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
CHECK-NEXT: 0
*)
