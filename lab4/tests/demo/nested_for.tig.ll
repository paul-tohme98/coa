; ModuleID = 'tiger'
source_filename = "tiger"

@0 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1

define i32 @main() {
entry:
  %j = alloca i32
  %i = alloca i32
  %k = alloca i32
  br label %body

body:                                             ; preds = %entry
  store i32 5, i32* %j
  store i32 0, i32* %i
  %0 = load i32, i32* %j
  br label %loop_test

loop_test:                                        ; preds = %loop_end3, %body
  %1 = load i32, i32* %i
  %2 = icmp sle i32 %1, %0
  br i1 %2, label %loop_body, label %loop_end

loop_body:                                        ; preds = %loop_test
  store i32 0, i32* %k
  %3 = load i32, i32* %j
  br label %loop_test1

loop_end:                                         ; preds = %loop_test
  ret i32 0

loop_test1:                                       ; preds = %loop_body2, %loop_body
  %4 = load i32, i32* %k
  %5 = icmp sle i32 %4, %3
  br i1 %5, label %loop_body2, label %loop_end3

loop_body2:                                       ; preds = %loop_test1
  %6 = load i32, i32* %i
  call void @__print_int(i32 %6)
  call void @__print(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @0, i32 0, i32 0))
  %7 = load i32, i32* %k
  %8 = add i32 %7, 1
  store i32 %8, i32* %k
  br label %loop_test1

loop_end3:                                        ; preds = %loop_test1
  %9 = load i32, i32* %i
  %10 = add i32 %9, 1
  store i32 %10, i32* %i
  br label %loop_test
}

declare void @__print_int(i32)

declare void @__print(i8*)
