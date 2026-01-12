MODULE Assert1;

BEGIN
  ASSERT(1X);
END Assert1.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
CHECK-NEXT: 0
*)
