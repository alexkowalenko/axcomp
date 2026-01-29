(*
RUN: %comp %s | filecheck %s
*)

module binary_mod;
begin
    return 4 mod 2;
end binary_mod.

(*
CHECK: 0
*)
