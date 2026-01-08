(*
RUN: %comp %s | echo $? | filecheck %s
CHECK: 0
*)

MODULE M;

PROCEDURE P(a: INTEGER): CHAR;
  END P;
  
END M.
