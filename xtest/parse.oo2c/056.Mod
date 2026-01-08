(*
RUN: %comp %s | echo $? | filecheck %s
CHECK: 0
*)

MODULE M;
TYPE T = POINTER TO R;
END M.
