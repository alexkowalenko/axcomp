(*
RUN: %comp %s | filecheck %s
*)

module binary_mult;
begin
    return 4 * 2;
end binary_mult.

(*
CHECK: 8
*)
