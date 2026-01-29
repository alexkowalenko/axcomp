MODULE Undef16;

IMPORT Out;

PROCEDURE P*(): INTEGER;
  VAR
    y, x: INTEGER;
    
  PROCEDURE N(): INTEGER;
    BEGIN
      RETURN x;
    END N;
  
  BEGIN
    y := N();
    x := 1;
    RETURN N();
  END P;

BEGIN
  Out.Int(P, 0); Out.Ln;
END Undef16.

(*
RUN: %comp %s | filecheck %s
CHECK: 1
CHECK-NEXT: 0
*)
