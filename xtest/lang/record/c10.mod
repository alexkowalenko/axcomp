MODULE c10; (* TYPE declarations *)

TYPE point = RECORD
          x,y,z : INTEGER;
      END;
VAR z0 : point;

PROCEDURE sum (a : point) : INTEGER;
BEGIN
     RETURN a.x + a.y + a.z
END sum;
    
BEGIN
     z0.x := 1;  z0.y := 1;  z0.z := 1;
     RETURN sum(z0)
END c10.

(*
RUN: %comp %s | filecheck %s
CHECK: 3
*)