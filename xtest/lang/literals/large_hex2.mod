(*
RUN: %comp %s | filecheck %s
CHECK: 35
*)

module large_hex2;
begin
    return 0cafe123H;
end large_hex2.
