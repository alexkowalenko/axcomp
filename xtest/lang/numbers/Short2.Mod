MODULE Short2;

PROCEDURE Test* ();
  VAR
    i8: SHORTINT;
  BEGIN
    i8 := 1;
    i8 := SHORT(i8);
  END Test;

BEGIN
  Test
END Short2.
