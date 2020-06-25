MODULE T8a;

IMPORT Out;

PROCEDURE f*(a, b, c: BOOLEAN): LONGINT;
  BEGIN
    IF (a & b) OR c THEN
      RETURN 9;
    ELSE
      RETURN -9;
    END;
  END f;

BEGIN
  Out.Int(f(FALSE, TRUE, TRUE), 0); Out.Ln;
END T8a.
