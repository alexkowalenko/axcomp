MODULE Const2;

IMPORT Out;

PROCEDURE Test* ();
  BEGIN
    IF FALSE THEN
      Out.String ("foo");
      RETURN;
    END;
    Out.String ("bar");
    Out.Ln;
  END Test;

BEGIN
  Test
END Const2.
