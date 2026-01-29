(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

module binary_mixed_4;

import Out;

begin
    Out.Real(2.4 + 2, 3); Out.Ln;
end binary_mixed_4.

(*
CHECK: 4.4
*)
