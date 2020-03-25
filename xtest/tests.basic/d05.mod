MODULE d05; (* Test top level VARs and procedure VARs *)
  VAR
        y :INTEGER;
    
  PROCEDURE f : INTEGER;
  VAR yy : INTEGER;
  BEGIN
      RETURN yy + 2;
  END f;

BEGIN
    RETURN y + 63;
END d05.