(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

module binary_mixed_2;

import Out;

begin
    Out.Real( 2 * 2.4, 3); Out.Ln;
end binary_mixed_2.

(*
CHECK: 4.8
*)
