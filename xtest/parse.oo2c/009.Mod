(*
RUN: %comp %s | echo $? | filecheck %s
CHECK: 0
*)

MODULE M;

CONST
  a = 1;
  b* = 1H;
  c* = 1X;

END M.
