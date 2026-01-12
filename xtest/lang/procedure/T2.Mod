MODULE T2;

IMPORT Out;

PROCEDURE f*(a, b: LONGINT): LONGINT;
  BEGIN
    RETURN a-b;
  END f;

BEGIN
  Out.Int(f(7, 4), 0); Out.Ln;
END T2.

(*
RUN: %comp %s | filecheck %s
CHECK: 3
CHECK-NEXT: 0
*)
