MODULE import04;
    
IMPORT beta;

BEGIN
    beta.c(24);
    RETURN 0; 
END import04.

(*
RUN: %ax -d beta.mod
RUN: %comp %s | filecheck %s
CHECK: 24
CHECK: 10
CHECK: 0
*)