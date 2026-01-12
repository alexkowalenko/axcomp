MODULE d06; (* Test return types from procedures *)
  VAR
     y :INTEGER;
    
  PROCEDURE f (): INTEGER;
  VAR yy : INTEGER;
  BEGIN
      RETURN yy + 2
  END f;

BEGIN
    RETURN y + 63
END d06.

(*
RUN: %comp %s | filecheck %s
CHECK: 63
*)
