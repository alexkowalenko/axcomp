MODULE Undef18;

IMPORT Out;

PROCEDURE P*(): INTEGER;
  VAR
    y, x: INTEGER;
    
  PROCEDURE N(VAR z: INTEGER): INTEGER;
    BEGIN
      RETURN z;
    END N;
  
  BEGIN
    y := N(x);
    x := 1;
    RETURN N(x);
  END P;

BEGIN
  Out.Int(P, 0); Out.Ln;
END Undef18.

(*
RUN: %comp %s | filecheck %s
CHECK: 1
CHECK-NEXT: 0
*)
