(*
RUN: %comp %s | echo $? | filecheck %s
CHECK: 0
*)

MODULE M;

PROCEDURE (r: SomePtr) P;
  END P;
  
END M.
