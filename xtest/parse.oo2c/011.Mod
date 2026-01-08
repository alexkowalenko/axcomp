(*
RUN: %comp %s | echo $? | filecheck %s
CHECK: 0
*)

MODULE M;

TYPE
  A = CHAR;
  B* = A;
  C* = INTEGER;
  D = M.X;
  
END M.
