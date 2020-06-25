MODULE T5;

IMPORT Out;

PROCEDURE f*(a, b: LONGINT): LONGINT;
  VAR
    h: LONGINT;
  BEGIN
    IF (b > a) THEN
      h := a; a := b; b := h;
    END;
    RETURN a-b;
  END f;

BEGIN
  Out.Int(f(3, 17), 0); Out.Ln;
END T5.
