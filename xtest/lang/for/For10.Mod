MODULE For10;

IMPORT Out;
  
PROCEDURE Test*;
  VAR
    x: INTEGER;
  BEGIN
    FOR x := 8 TO 1 BY -2 DO
      Out.Int (x, 0); Out.Ln;
    END;
  END Test;
  
BEGIN
  Test
END For10.
