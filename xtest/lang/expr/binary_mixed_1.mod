(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

module binary_mixed_1;

import Out;

begin
    Out.Real( 1 + 2.4, 3); Out.Ln;
end binary_mixed_1.

(*
CHECK: 3.4
*)
