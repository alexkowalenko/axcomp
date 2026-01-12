MODULE Return4;

BEGIN
  RETURN 123
END Return4.

(*
RUN: %comp %s | filecheck %s
CHECK: 123
*)
