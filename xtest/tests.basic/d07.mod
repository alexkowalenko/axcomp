MODULE d07; (* Test return types from procedures *)
  VAR
     y :INTEGER;
    
  PROCEDURE f (): INTEGER;
  VAR yy : INTEGER;
  BEGIN
      RETURN yy + 2
  END f;

BEGIN
    RETURN 5 + f() + (f() * f())
END d07.