MODULE Inc2;

VAR
  x: CHAR;
BEGIN
  INC(x,1);
  RETURN x;
END Inc2.

(*
RUN: %comp %s | filecheck %s
CHECK: 1
*)
