MODULE Alloc1;

IMPORT Out;

PROCEDURE F (a: INTEGER);
  BEGIN
    IF (a < 0) THEN
      Out.Int (a+1, 0);
    ELSE
      Out.Int (a-1, 0);
    END;
    Out.Ln;
  END F;

PROCEDURE Test* ();
  BEGIN
    F(-1);
    F(1);
  END Test;

BEGIN
  Test;
END Alloc1.
