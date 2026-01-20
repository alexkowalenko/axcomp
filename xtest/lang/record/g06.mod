MODULE g06; (* Records *)

VAR x1 : RECORD 
        x, y, z : INTEGER;
        a : BOOLEAN
        j : ARRAY 3 OF INTEGER;
    END;

PROCEDURE f;
    VAR x1 : RECORD 
        x, y, z : INTEGER;
    END; 
BEGIN
    RETURN
END f;

BEGIN
    f();
    RETURN 0
END g06.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)