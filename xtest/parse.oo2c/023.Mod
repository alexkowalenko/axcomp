(*
RUN: %comp %s | echo $? | filecheck %s
CHECK: 0
*)

MODULE M;

PROCEDURE P(a, b: INTEGER; c, d: BOOLEAN): CHAR;
  END P;
  
END M.
