MODULE g11; (* ARRAY parameters and return values *)
        
VAR pt: ARRAY 3 OF INTEGER;
    i: INTEGER;

PROCEDURE identity(a : ARRAY 3 OF INTEGER) : ARRAY 3 OF INTEGER;
BEGIN
    RETURN a
END identity;

PROCEDURE sum(a : ARRAY 3 OF INTEGER) : INTEGER;
VAR total,i : INTEGER;
BEGIN
    FOR i := 0 TO 2 DO
        total := total + a[i]
    END;
    RETURN total
END sum;

BEGIN
    FOR i := 0 TO 2 DO
        pt[i] := i*i + i + 1
    END;
    RETURN sum(identity(pt))
END g11.

(*
RUN: %comp %s | filecheck %s
CHECK: 11
*)