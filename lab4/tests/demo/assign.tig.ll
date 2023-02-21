; ModuleID = 'tiger'
source_filename = "tiger"

define i32 @main() {
entry:
  %a = alloca i32
  br label %body

body:                                             ; preds = %entry
  store i32 1, i32* %a
  store i32 3, i32* %a
  %0 = load i32, i32* %a
  call void @__print_int(i32 %0)
  ret i32 0
}

declare void @__print_int(i32)
