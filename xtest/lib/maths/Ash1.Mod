MODULE Ash1;

IMPORT Out;

PROCEDURE Ash (x, n: INTEGER; res: LONGINT);
  BEGIN
    Out.Int(x, 4); Out.Char(","); Out.Int(n, 0); Out.String("  ");
    Out.Int(res, 4);
    Out.Int(ASH(x,n), 4);
    Out.Ln;
  END Ash;

PROCEDURE Test* ();
  VAR
    si: SHORTINT;
  BEGIN
    Ash(0, 0, ASH(0,0));
    Ash(0, 1, ASH(0,1));
    Ash(0, -1, ASH(0,-1));
    
    Ash(1, 0, ASH(1,0));
    Ash(1, 1, ASH(1,1));
    Ash(1, -1, ASH(1,-1));
    
    Ash(-1, 0, ASH(-1,0));
    Ash(-1, 1, ASH(-1,1));
    Ash(-1, -1, ASH(-1,-1));

    si := 1;
    Ash(si, 8, ASH(si,8));
  END Test;

BEGIN
  Test
END Ash1.

(*
RUN: %comp %s | filecheck %s
CHECK:    0,0     0   0
CHECK-NEXT:          0,1     0   0
CHECK-NEXT:0,-1     0   0
CHECK-NEXT:1,0     1   1
CHECK-NEXT:1,1     2   2
CHECK-NEXT:1,-1  -9223372036854775808-9223372036854775808
CHECK-NEXT:-1,0    -1  -1
CHECK-NEXT:-1,1    -2  -2
CHECK-NEXT:-1,-1  -9223372036854775808-9223372036854775808
CHECK-NEXT:1,8   256 256
CHECK-NEXT: 0
*)
