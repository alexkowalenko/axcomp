MODULE For4;

PROCEDURE Test*;
  VAR
    x: SHORTINT;
  BEGIN
    FOR x := 128 TO 127 DO
      WriteInt(x); WriteLn;
    END;
  END Test;
  
BEGIN
  Test
END For4.
