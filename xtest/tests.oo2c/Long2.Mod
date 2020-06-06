MODULE Long2;

PROCEDURE Test* ();
  VAR
    i64: HUGEINT;
  BEGIN
    i64 := 1;
    i64 := LONG(i64);
  END Test;

BEGIN
  Test
END Long2.
