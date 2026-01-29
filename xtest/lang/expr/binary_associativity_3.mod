(*
RUN: %comp %s | filecheck %s
*)

module binary_associativity_3;
begin
    return (3 DIV 2 * 4) + (5 - 4 + 3);
end binary_associativity_3.

(*
CHECK: 8
*)
