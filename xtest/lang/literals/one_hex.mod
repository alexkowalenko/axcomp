(*
RUN: %comp %s | filecheck %s
*)

module one_hex;
begin
    return 1H;
end one_hex.

(*
CHECK: 1
*)