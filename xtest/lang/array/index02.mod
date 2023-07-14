MODULE index02;
VAR 
    a : ARRAY 3 OF INTEGER;
    x : INTEGER;
BEGIN
    a[0] := 7;
    x := a[0];
    WriteInt(x); WriteLn;
END index02.