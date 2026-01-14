MODULE Inc4;

VAR
  x: SHORTINT;
BEGIN
  INC(x, 128);
  RETURN x
END Inc4.

(*
RUN: %comp %s | filecheck %s
CHECK: 128
*)
