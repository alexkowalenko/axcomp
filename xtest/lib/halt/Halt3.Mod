MODULE Halt3;

VAR
  x: INTEGER;
BEGIN
  x := 123;
  HALT(x);
END Halt3.
(*
RUN: %comp %s | filecheck %s
CHECK: 1
*)
