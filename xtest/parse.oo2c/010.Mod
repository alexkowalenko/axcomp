(*
RUN: %comp %s | echo $? | filecheck %s
CHECK: 0
*)

MODULE M;

CONST

END M.
