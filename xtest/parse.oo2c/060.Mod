(*
RUN: %comp %s | echo $? | filecheck %s
CHECK: 0
*)

MODULE M;
TYPE T = RECORD (Base) END;
END M.
