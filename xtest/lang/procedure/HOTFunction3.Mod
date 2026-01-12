MODULE HOTFunction3;

IMPORT Out;
  
PROCEDURE Identity (n: INTEGER): INTEGER;
  VAR
    x: INTEGER;
  BEGIN
    x := n;
    RETURN x
  END Identity;

PROCEDURE Test* ();
  BEGIN
    Out.Int(Identity(1), 0); Out.Ln;
    Out.Int(Identity(2), 0); Out.Ln;
  END Test;

BEGIN
  Test
END HOTFunction3.

(*
RUN: %comp %s | filecheck %s
CHECK: 1
CHECK-NEXT: 2
CHECK-NEXT: 0
*)
