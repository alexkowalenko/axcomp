MODULE NestedProc2;

IMPORT Out;
  
PROCEDURE Test* ();
  VAR
    x: INTEGER;
    
  PROCEDURE Inner ();
    BEGIN
      x := 123;
    END Inner;
  
  BEGIN
    x := -1;
    Inner;
    Out.Int (x, 0); Out.Ln;
  END Test;

BEGIN
  Test
END NestedProc2.
