MODULE alias04;
    
IMPORT B := beta;

BEGIN
    B.c(24);
    RETURN 0; 
END alias04.


(*
RUN: %ax -d beta.mod
RUN: %comp %s | filecheck %s
CHECK: 24
CHECK: 10
CHECK: 0
*)