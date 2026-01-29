
(*
RUN: %comp %s | filecheck %s
*)

MODULE compare_numeric;
BEGIN
    WriteBoolean(12 >= 18); WriteLn;
    WriteBoolean(12 <= 18); WriteLn;
    WriteBoolean(12 = 18); WriteLn;
    WriteBoolean(12 # 18); WriteLn; 
END compare_numeric.

(*
CHECK: 0
CHECK-NEXT: 1
CHECK-NEXT: 0
CHECK-NEXT: 1
*)
