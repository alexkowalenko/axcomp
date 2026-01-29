(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

module binary_real_5;

import Out;

begin
    Out.Real( - 3.1415927 + 2.1819 / (1.2 - 0.1 * 0.9), 6 ); Out.Ln;
end binary_real_5.

(*
CHECK: -1.17592
*)
