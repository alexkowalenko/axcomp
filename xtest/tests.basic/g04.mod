MODULE g04; (* Arrays *)

VAR x : ARRAY [3] OF BOOLEAN;

BEGIN
    x[0] := FALSE;
    x[1] := TRUE;
    x[2] := FALSE;
    FOR i := 0 TO 2 DO
        WriteInt(x[i]); WriteLn();
    END;
    RETURN x[0] OR x[1] OR x[2];
END g04.