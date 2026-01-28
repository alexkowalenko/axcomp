(*
RUN: %comp %s | filecheck %s
Can't return large numbers directly yet.
CHECK: 52
*)

module large_hex3;
begin
    return 0ffff1234H;
end large_hex3.
