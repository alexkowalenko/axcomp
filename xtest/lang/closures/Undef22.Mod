MODULE Undef22;

IMPORT Out;

PROCEDURE P*(): INTEGER;
  VAR
    y, x: INTEGER;

  PROCEDURE Q(): INTEGER;
    PROCEDURE N(VAR z: INTEGER): INTEGER;
      BEGIN
        RETURN z;
      END N;
    
    BEGIN
      RETURN N(x);
    END Q;
  
  BEGIN
    y := Q();
    x := 1;
    RETURN Q();
  END P;

BEGIN
  Out.Int(P, 0); Out.Ln;
END Undef22.

(*
RUN: %comp %s | filecheck %s
CHECK: 1
CHECK-NEXT: 0
*)
