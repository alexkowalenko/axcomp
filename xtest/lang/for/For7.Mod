MODULE For7;

PROCEDURE Test*;
  VAR
    x, y: SHORTINT;
  BEGIN
    y := 1;
    FOR x := 0 TO 127 BY y DO
       WriteInt(x); WriteLn;
    END;
  END Test;
  
BEGIN
  Test
END For7.
