MODULE Undef7;

IMPORT Out;

PROCEDURE P*(b: BOOLEAN): INTEGER;
  VAR
    x: INTEGER;
  BEGIN
    LOOP
      IF b THEN
        x := 1;
        EXIT;
      ELSE
        x := 2;
        EXIT;
      END;
    END;
    RETURN x;
  END P;

BEGIN
  Out.Int(P(TRUE), 0); Out.Ln;
END Undef7.
