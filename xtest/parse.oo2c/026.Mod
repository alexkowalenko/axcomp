(*
RUN: %comp %s | echo $? | filecheck %s
CHECK: 0
*)

MODULE M;

PROCEDURE ^ (VAR r: SomeRec) P;
  
END M.
