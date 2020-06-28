MODULE MagicSquares;   (*for Oberon-0 NW 25.1.2013*)
IMPORT Out;
VAR count : INTEGER;

PROCEDURE Generate(n: INTEGER);  (*magic square of order 3, 5, 7, ... *)
    VAR i, j, x, nx, nsq: INTEGER;
      M: ARRAY 13, 13 OF INTEGER;
  BEGIN 
      nsq := n*n; x := 0;
      i := n DIV 2; j := n-1;
      WHILE x < nsq DO
        nx := n + x; j := (j-1) MOD n; x := x+1; M[i, j] := x;
        WHILE x < nx DO
          i := (i+1) MOD n; j := (j+1) MOD n;
          x := x+1; M[i, j] := x
        END
      END ;
    i := 0;
    REPEAT j := 0;
      REPEAT Out.Int(M[i, j], 2); Out.Char(' '); j := j+1 UNTIL j = n;
      Out.Ln; i := i+1
    UNTIL i = n
  END Generate;

BEGIN 
    FOR count := 3 TO 9 BY 2 DO
      Generate(count);
      Out.Ln;
    END
END MagicSquares.