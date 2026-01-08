(*
RUN: %comp %s | echo $? | filecheck %s
CHECK: 0
*)

MODULE M;

VAR
  a, b-, c*: INTEGER;
  
END M.
