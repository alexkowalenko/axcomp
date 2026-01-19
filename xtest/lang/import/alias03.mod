MODULE alias03;
    
IMPORT  B:= beta;

BEGIN
    B.d := TRUE;
    WriteBoolean(B.d); WriteLn;
    RETURN 0; 
END alias03.

(*
RUN: %ax -d beta.mod
RUN: %comp %s | filecheck %s
CHECK: 1
CHECK: 0
*)