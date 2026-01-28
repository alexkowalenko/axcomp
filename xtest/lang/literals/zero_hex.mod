(*
RUN: %comp %s | filecheck %s
*)

module zero_hex;
begin
    return 0H;
end zero_hex.


(*
CHECK: 0
*)