MODULE import01;
    
IMPORT beta;

BEGIN
    RETURN beta.a;
END import01.

(*
RUN: %ax -d beta.mod
RUN: %comp %s beta.o | filecheck %s
CHECK: 3
*)