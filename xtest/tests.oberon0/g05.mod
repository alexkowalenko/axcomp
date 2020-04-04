MODULE g05; (* Arrays *)

VAR a : ARRAY 3 OF ARRAY 3 OF INTEGER;
    sum : INTEGER;

BEGIN
    FOR y := 0 TO 2 DO
        FOR x := 0 TO 2 DO
            a[x][y] := x + y
        END
    END;

     FOR y := 0 TO 2 DO
        FOR x := 0 TO 2 DO
            WriteInt(a[x][y]);
            sum := sum + 1
        END;
        WriteLn()
    END;
      WriteInt(sum);   WriteLn();
    RETURN 0
END g05.