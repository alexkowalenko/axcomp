(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

module binary_mixed_3;

import Out;

begin
    Out.Real( 2 / 2.4, 6); Out.Ln;
end binary_mixed_3.

(*
CHECK: 0.833333
*)
