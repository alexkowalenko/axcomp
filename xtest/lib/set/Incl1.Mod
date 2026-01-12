MODULE Incl1;

IMPORT Out;
  
PROCEDURE Test* ();
  VAR
    s: SET;
  BEGIN
    s := {};
    INCL(s,0);
    Out.Set(s); Out.Ln;
    INCL(s,31);
    Out.Set(s); Out.Ln;
    EXCL(s,0);
    Out.Set(s); Out.Ln;
    EXCL(s,31);
    Out.Set(s); Out.Ln;
  END Test;

BEGIN
  Test
END Incl1.

(*
RUN: %comp %s | filecheck %s
CHECK: {0}
CHECK-NEXT: {0,31}
CHECK-NEXT: {31}
CHECK-NEXT: {}
*)
