MODULE Dead1;

PROCEDURE P();
  VAR
    i, j: INTEGER;
  BEGIN
    j := 123;
    FOR i := 0 TO 9 DO
      INC(j);
    END;
    WriteInt(j); WriteLn;
  END P;

BEGIN
  P;
END Dead1.
