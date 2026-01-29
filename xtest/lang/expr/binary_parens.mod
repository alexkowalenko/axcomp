(*
RUN: %comp %s | filecheck %s
*)

module binary_parens;
begin
    return 2 * (3 + 4);
end binary_parens.

(*
CHECK: 14
*)
