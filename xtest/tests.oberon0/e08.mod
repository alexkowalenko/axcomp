MODULE e08; (* BEGIN block *)

VAR x : INTEGER;

BEGIN
    x := 0;
    BEGIN
        x := x + 1;
        x := x + 1;
        x := x + 1
    END;
    RETURN x
END e08.