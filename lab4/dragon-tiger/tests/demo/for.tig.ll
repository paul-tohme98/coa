; ModuleID = 'tiger'
source_filename = "tiger"

@0 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1

define i32 @main() {
entry:
  %j = alloca i32
  %i = alloca i32
  br label %body

body:                                             ; preds = %entry
  store i32 10, i32* %j
  store i32 0, i32* %i
  %0 = load i32, i32* %j
  br label %loop_test

loop_test:                                        ; preds = %loop_body, %body
  %1 = load i32, i32* %i
  %2 = icmp sle i32 %1, %0
  br i1 %2, label %loop_body, label %loop_end

loop_body:                                        ; preds = %loop_test
  %3 = load i32, i32* %i
  call void @__print_int(i32 %3)
  call void @__print(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @0, i32 0, i32 0))
  %4 = load i32, i32* %i
  %5 = add i32 %4, 1
  store i32 %5, i32* %i
  br label %loop_test

loop_end:                                         ; preds = %loop_test
  ret i32 0
}

declare void @__print_int(i32)

declare void @__print(i8*)
