MODULE Len8;
IMPORT Out;
TYPE
  R = RECORD END;

PROCEDURE F(): LONGINT;
  VAR
    r: R;
  BEGIN
    RETURN LEN(r);
  END F;

BEGIN
  Out.Int(F(), 0); Out.Ln;  
END Len8.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
CHECK-NEXT: 0
*)
