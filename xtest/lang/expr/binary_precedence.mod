(*
RUN: %comp %s | filecheck %s
*)

module binary_precedence;
begin
    return 2 + 3 * 4;
end binary_precedence.

(*
CHECK: 14
*)
