MODULE C1;

IMPORT Out;

PROCEDURE Sub(a, b: LONGINT): LONGINT;
  BEGIN
    RETURN a-b;
  END Sub;

PROCEDURE f*(a, b: LONGINT): LONGINT;
  BEGIN
    RETURN Sub(a, b);
  END f;


BEGIN 
  Out.Int(f(5,2), 0); Out.Ln;
END C1.

(*
RUN: %comp %s | filecheck %s
CHECK: 3
CHECK-NEXT: 0
*)
