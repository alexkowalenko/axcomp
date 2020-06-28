MODULE Undef4;

IMPORT Out;

PROCEDURE P*(b: BOOLEAN): INTEGER;
  VAR
    x: INTEGER;
  BEGIN
    IF b THEN
    ELSE
      x := 1;
    END;
    RETURN x;
  END P;

BEGIN
  Out.Int(P(TRUE), 0); Out.Ln;
END Undef4.
