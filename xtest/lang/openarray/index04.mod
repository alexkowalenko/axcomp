MODULE index04; (* open arrays *)
IMPORT Out;
VAR x : ARRAY OF INTEGER;
    sum, i: INTEGER;
BEGIN
  
    NEW(x, 10);
    x[1] := 7;
  
    (*
  
    FOR i := 0 TO 9 DO
        x[i] := i*i + i + 1;
    END
    FOR i := 0 TO 9 DO
        Out.Int(x[i], 2) Out.Char(' ');
    END
    *)
    Out.Ln;

END index04.