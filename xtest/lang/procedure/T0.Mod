MODULE T0;

IMPORT Out;

PROCEDURE f*(a: LONGINT): LONGINT;
  BEGIN
    RETURN a;
  END f;

BEGIN
  Out.Int(f(3), 0); Out.Ln;
END T0.

(*
RUN: %comp %s | filecheck %s
CHECK: 3
CHECK-NEXT: 0
*)
