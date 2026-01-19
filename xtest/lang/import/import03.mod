MODULE import03;
    
IMPORT beta;

BEGIN
    beta.d := TRUE;
    WriteBoolean(beta.d); WriteLn;
    RETURN 0; 
END import03.

(*
RUN: %ax -d beta.mod
RUN: %comp %s | filecheck %s
CHECK: 1
CHECK: 0
*)