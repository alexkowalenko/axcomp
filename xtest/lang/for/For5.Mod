MODULE For5;

PROCEDURE Test*;
  VAR
    x: SHORTINT;
  BEGIN
    FOR x := 127 TO 128 DO
       WriteInt(x); WriteLn;
    END;
  END Test;
  
BEGIN
  Test
END For5.
