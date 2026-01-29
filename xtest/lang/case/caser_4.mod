(*
RUN:  %comp %s | filecheck %s
*)

<* MAIN+ *>

module caser_4;
var x: INTEGER;
begin
    x := 8;
    case x of
      1: WriteInt(1); WriteLn;
      | 2: WriteInt(2); WriteLn;
      | 5,3: WriteInt(3); WriteLn;
      | 4:  WriteInt(4); WriteLn;
    end;
    WriteBoolean(TRUE); WriteLn;
end caser_4.

(*
CHECK: 1
*)

