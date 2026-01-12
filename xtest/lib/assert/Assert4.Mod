MODULE Assert4;

BEGIN
  ASSERT(TRUE, -1);
END Assert4.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)
