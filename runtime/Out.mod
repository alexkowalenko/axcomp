(* 
* AX compiler
*
* Copyright © 2020 Alex Kowalenko
*
* Runtime library: Standard Output
*)

MODULE Out; 

PROCEDURE Open*;
END Open;

PROCEDURE Flush*;
END Flush;

PROCEDURE Int* (i, n: INTEGER);
END Int;

PROCEDURE Hex*(x,  n: INTEGER);
END Hex;

PROCEDURE Bool*(x : BOOLEAN);
END Bool;

PROCEDURE Real*(x : REAL; n: INTEGER);
END Real;

PROCEDURE Char*(x : CHAR);
END Char;

PROCEDURE String*(x : STRING);
END String;

PROCEDURE Ln*;
END Ln;

END Out.