MODULE import02;
    
IMPORT beta;

BEGIN
    beta.b := 30;
    RETURN beta.b; 
END import02.

(*
RUN: %ax -d beta.mod
RUN: %comp %s beta.o | filecheck %s
CHECK: 30
*)