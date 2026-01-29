(*
RUN: %comp %s | filecheck %s
*)

module binary_associativity_2;
begin
    return 6 div 3 DIV 2;
end binary_associativity_2.

(*
CHECK: 1
*)
