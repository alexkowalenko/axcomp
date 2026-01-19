MODULE alias01;
    
IMPORT B := beta;

BEGIN
    RETURN B.a; 
END alias01.

(*
RUN: %ax -d beta.mod
RUN: %comp %s | filecheck %s
CHECK: 3
*)