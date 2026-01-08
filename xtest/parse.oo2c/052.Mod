(*
RUN: %comp %s | echo $? | filecheck %s
CHECK: 0
*)

MODULE M;
TYPE T = ARRAY 10 OF CHAR;
END M.
