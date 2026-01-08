(*
RUN: %comp %s | echo $? | filecheck %s
CHECK: 0
*)

MODULE M;

PROCEDURE P(VAR a, b: INTEGER): CHAR;
  END P;
  
END M.
