; ModuleID = 'Const5'
source_filename = "Const5"

@STRING_0 = private global [0 x i32] [i32 102, i32 111, i32 111, i32 0]
@STRING_1 = private global [0 x i32] [i32 98, i32 97, i32 114, i32 0]

declare void @Out_Ln()

declare void @Out_String([0 x i32]*)

define void @Const5_Test() {
entry:
  switch i64 10, label %range [
  ]

range:                                            ; preds = %entry
  br label %case.range0

case.range0:                                      ; preds = %range
  br i1 false, label %case.element0, label %case_end

case.element0:                                    ; preds = %case.range0
  call void @Out_String([0 x i32]* @STRING_0)
  ret void

case_end:                                         ; preds = %case.range0
  call void @Out_String([0 x i32]* @STRING_1)
  call void @Out_Ln()
}

define i64 @output() {
entry:
  call void @Const5_Test()
  ret i64 0
}
