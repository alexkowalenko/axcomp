MODULE ConstFold7;

IMPORT Out;

TYPE
  A = ARRAY 16 OF CHAR;

VAR
  a: A;
  
PROCEDURE Test*;
  BEGIN
    Out.Int(LEN (a),0); Out.Ln;
  END Test;

BEGIN
  Test
END ConstFold7.
