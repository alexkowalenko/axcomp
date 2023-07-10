MODULE c07; (* vars *)
CONST
    alpha = 24;
VAR
    x : INTEGER;
    y : BOOLEAN;
BEGIN
   y := FALSE;
   x := 3 * alpha;
   RETURN ~ (x = 72)
END c07.