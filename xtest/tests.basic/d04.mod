MODULE d04; (* Test top level consts and procedure consts *)
  CONST
        y = 3;
    
  PROCEDURE f : INTEGER;
  CONST y = 4; (* shadow global const *)
  BEGIN
      RETURN y
  END f;

BEGIN
    RETURN y
END d04.