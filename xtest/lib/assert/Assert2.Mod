MODULE Assert2;

BEGIN
  ASSERT(TRUE, 1X);
END Assert2.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)
