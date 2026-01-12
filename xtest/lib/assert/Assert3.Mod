MODULE Assert3;

VAR
  x: INTEGER;
BEGIN
  x := 123;
  ASSERT(TRUE, x);
END Assert3.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)
