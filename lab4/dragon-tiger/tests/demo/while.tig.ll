; ModuleID = 'tiger'
source_filename = "tiger"

define i32 @main() {
entry:
  %j = alloca i32
  br label %body

body:                                             ; preds = %entry
  store i32 10, i32* %j
  br label %loop_test

loop_test:                                        ; preds = %loop_body, %body
  %0 = load i32, i32* %j
  %1 = icmp sgt i32 %0, 0
  %2 = zext i1 %1 to i32
  %3 = icmp ne i32 %2, 0
  br i1 %3, label %loop_body, label %loop_end

loop_body:                                        ; preds = %loop_test
  %4 = load i32, i32* %j
  call void @__print_int(i32 %4)
  br label %loop_test

loop_end:                                         ; preds = %loop_test
  %5 = load i32, i32* %j
  %6 = load i32, i32* %j
  %7 = add i32 %6, 1
  %8 = icmp eq i32 %5, %7
  %9 = zext i1 %8 to i32
  ret i32 0
}

declare void @__print_int(i32)
