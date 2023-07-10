MODULE Abs2;

PROCEDURE Test* ();
  VAR
    x: INTEGER;
  BEGIN
    x := ABS(0X);
    WriteInt(x); WriteLn;
  END Test;

BEGIN
  Test;
END Abs2.
