(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)

MODULE ConstFold2;

IMPORT Out;
  
CONST c=1+2;

PROCEDURE Test* ();
  BEGIN
    Out.Int (c, 0); Out.Ln;
  END Test;

BEGIN
  Test
END ConstFold2.

