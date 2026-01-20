MODULE Undef2;

IMPORT Out;

TYPE
  R = RECORD
    a*, b*: INTEGER;
  END;
  
PROCEDURE P*(): INTEGER;
  VAR
    r: R;
  BEGIN
    r.a := 1;
    RETURN r.a;
  END P;

BEGIN
  Out.Int(P, 0); Out.Ln;
END Undef2.


(*
RUN: %comp %s | filecheck %s
CHECK: 1
CHECK-NEXT: 0
*)
