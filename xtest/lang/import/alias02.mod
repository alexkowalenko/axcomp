MODULE alias02;
    
IMPORT B := beta;

BEGIN
    B.b := 30;
    RETURN B.b; 
END alias02.

(*
RUN: %ax -d beta.mod
RUN: %comp %s | filecheck %s
CHECK: 30
*)