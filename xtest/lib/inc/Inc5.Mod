MODULE Inc5;

VAR
  x: SHORTINT;
  y: INTEGER;
BEGIN
  INC(x, y);
  RETURN x
END Inc5.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)
