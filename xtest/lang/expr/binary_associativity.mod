(*
RUN: %comp %s | filecheck %s
*)

module binary_associativity;
begin
    return 1 - 2 - 3;
end binary_associativity.

(* -4
CHECK: 252
*)
