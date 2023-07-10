MODULE Undef5;

IMPORT Out;

PROCEDURE P*(b: BOOLEAN): INTEGER;
  VAR
    x: INTEGER;
  BEGIN
    IF b THEN
      x := 1;
    ELSE
      x := 2;
    END;
    RETURN x;
  END P;

BEGIN
  Out.Int(P(TRUE), 0); Out.Ln;
END Undef5.
