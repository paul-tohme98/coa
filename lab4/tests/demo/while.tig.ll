; ModuleID = 'tiger'
source_filename = "tiger"

define i32 @main() {
entry:
  %j = alloca i32
  br label %body

body:                                             ; preds = %entry
  store i32 10, i32* %j
  br label %loop_test

loop_test:                                        ; preds = %if_end, %body
  %0 = load i32, i32* %j
  %1 = icmp sgt i32 %0, 0
  %2 = zext i1 %1 to i32
  %3 = icmp ne i32 %2, 0
  br i1 %3, label %loop_body, label %loop_end

loop_body:                                        ; preds = %loop_test
  %4 = load i32, i32* %j
  %5 = icmp slt i32 %4, 5
  %6 = zext i1 %5 to i32
  %7 = icmp ne i32 %6, 0
  br i1 %7, label %if_then, label %if_else

loop_end:                                         ; preds = %loop_test
  %8 = load i32, i32* %j
  %9 = load i32, i32* %j
  %10 = add i32 %9, 1
  %11 = icmp eq i32 %8, %10
  %12 = zext i1 %11 to i32
  ret i32 0

if_then:                                          ; preds = %loop_body
  br label %br_block

if_else:                                          ; preds = %loop_body
  %13 = load i32, i32* %j
  call void @__print_int(i32 %13)
  br label %if_end

if_end:                                           ; preds = %if_else, %br_block
  br label %loop_test

br_block:                                         ; preds = %if_then
  br label %if_end
}

declare void @__print_int(i32)
