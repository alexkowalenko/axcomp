
(*
RUN: %comp %s | filecheck %s
*)

MODULE compare_numeric_2;
BEGIN
    WriteBoolean(1 + 12 >= 18); WriteLn;
    WriteBoolean(12 <= 18 + 23); WriteLn;
    WriteBoolean(12 * 12 = 18); WriteLn;
    WriteBoolean(12 # 18 MOD 3); WriteLn; 
END compare_numeric_2.

(*
CHECK: 0
CHECK-NEXT: 1
CHECK-NEXT: 0
CHECK-NEXT: 1
*)
