MODULE len; (* LEN function*)
IMPORT Out;
VAR x : INTEGER;
    y : ARRAY 7 OF INTEGER;

BEGIN
     Out.Int(LEN(y)); Out.Ln;
     FOR i := 0 TO LEN(y) - 1 DO
       y[i] := i*i;
     END;
      FOR i := 0 TO LEN(y) - 1 DO
        Out.Int(y[i]); Out.Ln;
     END;
    
END len.