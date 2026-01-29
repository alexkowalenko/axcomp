(*
RUN: %comp %s | filecheck %s
*)

module binary_add;
begin
    return 7 + 4;
end binary_add.

(*
CHECK: 11
*)
