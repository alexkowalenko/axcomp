(*
RUN: %comp %s | echo $? | filecheck %s
CHECK: 0
*)

MODULE M;

VAR
  a: INTEGER;
  b-: CHAR;
  c*: LONGREAL;
  
END M.
