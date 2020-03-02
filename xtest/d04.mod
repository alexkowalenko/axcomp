MODULE d04; (* Test top level consts and procedure consts *)
  CONST
        y = 3;
    
  PROCEDURE f;
  CONST yy = 4;
  BEGIN
      RETURN yy;
  END f;

BEGIN
    RETURN y;
END d04.