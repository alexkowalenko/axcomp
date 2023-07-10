MODULE e05; (* test ELSIF branch *)
VAR x : INTEGER;
BEGIN
    x := 2;
    IF x = 1 THEN
        x := 10
    ELSIF x = 2 THEN
        x := 20
    ELSIF x = 3 THEN
        x := 30
    ELSE
        x := 90
    END; (* x should be 20 *)

    IF x = 10 THEN
        x := 100
    ELSIF x = 30 THEN
        x := 300
    ELSIF x = 20 THEN
        x := 200
    ELSE
        x := 900
    END; 

    RETURN x (* x should be 200 *)
END e05.