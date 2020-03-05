MODULE d08; (* Test procedure parameters *)
  VAR
     y :INTEGER;
    
  PROCEDURE f (x : INTEGER): INTEGER;
  VAR yy : INTEGER;
  BEGIN
      RETURN yy + 2;
  END f;

BEGIN
    RETURN 5 + f() + (f() * f());
END d08.