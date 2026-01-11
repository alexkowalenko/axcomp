(*
RUN: %comp %s | filecheck %s
CHECK: 20
CHECK-NEXT: 0
*)

MODULE ConstFold17;

IMPORT Out;

PROCEDURE Test*;
BEGIN
  Out.Real(10.0*2.0, 3);
  Out.Ln;
END Test;

BEGIN
  Test
END ConstFold17.
