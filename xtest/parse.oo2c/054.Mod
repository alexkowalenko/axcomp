(*
RUN: %comp %s | echo $? | filecheck %s
CHECK: 0
*)

MODULE M;
TYPE T = ARRAY OF ARRAY 10 OF CHAR;
END M.
