MODULE c05; (* vars *)
CONST
    alpha = 24;
VAR
    x : INTEGER;
    y : INTEGER;
BEGIN
    alpha * x; (* should be 0 as vars are initialised to 0 *)
END c05.