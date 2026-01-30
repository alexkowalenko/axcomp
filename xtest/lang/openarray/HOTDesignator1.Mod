MODULE HOTDesignator1;

IMPORT Out;
  
PROCEDURE Get (str: ARRAY OF CHAR; n: INTEGER): CHAR;
  BEGIN
    RETURN str[n]
  END Get;

PROCEDURE Test* ();
  BEGIN
    Out.Char(Get("ab", 0)); Out.Ln;
    Out.Char(Get("ab", 1)); Out.Ln;
    Out.Char(Get("ab", 2)); Out.Ln;
  END Test;

BEGIN
  Test
END HOTDesignator1.

(*
RUN: %comp %s | filecheck %s
CHECK: a
CHECK-NEXT: b
CHECK: 0
*)
