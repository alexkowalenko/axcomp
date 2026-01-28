
(*
RUN: %comp %s | filecheck %s
*)

MODULE large_int;
BEGIN
    WriteInt(1FFFFH); WriteLn;    
    RETURN 0; 
END large_int.

(*
CHECK: 131071
CHECK-NEXT: 0
*)
