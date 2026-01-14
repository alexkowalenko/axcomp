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

(*
RUN: %comp %s | filecheck %s
CHECK: 3  8  0
CHECK-NEXT:       5  1  9
CHECK-NEXT:       7  6  2
CHECK-EMPTY:
CHECK-NEXT:      10  4 23 17  0
CHECK-NEXT:      12  6  5 24 18
CHECK-NEXT:      19 13  7  1 25
CHECK-NEXT:      21 20 14  8  2
CHECK-NEXT:       3 22 16 15  9
CHECK-EMPTY:
CHECK-NEXT:      21 13  5 46 38 30  0
CHECK-NEXT:      23 15 14  6 47 39 31
CHECK-NEXT:      32 24 16  8  7 48 40
CHECK-NEXT:      41 33 25 17  9  1 49
CHECK-NEXT:      43 42 34 26 18 10  2
CHECK-NEXT:       3 44 36 35 27 19 11
CHECK-NEXT:      12  4 45 37 29 28 20
CHECK-EMPTY:
CHECK-NEXT:      36 26 16  6 77 67 57 47  0
CHECK-NEXT:      38 28 27 17  7 78 68 58 48
CHECK-NEXT:      49 39 29 19 18  8 79 69 59
CHECK-NEXT:      60 50 40 30 20 10  9 80 70
CHECK-NEXT:      71 61 51 41 31 21 11  1 81
CHECK-NEXT:      73 72 62 52 42 32 22 12  2
CHECK-NEXT:       3 74 64 63 53 43 33 23 13
CHECK-NEXT:      14  4 75 65 55 54 44 34 24
CHECK-NEXT:      25 15  5 76 66 56 46 45 35
CHECK-EMPTY:
CHECK-NEXT: 0
*)
