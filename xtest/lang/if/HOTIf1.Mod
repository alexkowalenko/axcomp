MODULE HOTIf1;

IMPORT Out;
  
PROCEDURE If (b: BOOLEAN);
  VAR
    x: INTEGER;
  BEGIN
    IF b THEN
      Out.String ("TRUE")
    ELSE
      Out.String ("FALSE")
    END;
    Out.Ln
  END If;

PROCEDURE Test* ();
  BEGIN
    ;If (TRUE);;
    If (FALSE);
  END Test;

BEGIN
  Test
END HOTIf1.
