MODULE Div1;

IMPORT Out;

PROCEDURE Test* ();
  VAR
    i: INTEGER;
    x: INTEGER;
  BEGIN
    FOR i := -6 TO 6 DO
      Out.Int (i, 3); Out.String (" DIV "); Out.Int (3, 0); Out.String (" = ");
      x := i DIV 3;
      Out.Int (x, 0); Out.Ln;
    END;
  END Test;

BEGIN
  Test
END Div1.
