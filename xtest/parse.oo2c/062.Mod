(*
RUN: %comp %s | echo $? | filecheck %s
CHECK: 0
*)

MODULE M;
TYPE T = RECORD a, b-, c*: CHAR END;
END M.
