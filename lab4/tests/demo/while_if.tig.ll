; ModuleID = 'tiger'
source_filename = "tiger"

@0 = private unnamed_addr constant [4 x i8] c"no\0A\00", align 1

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
  %5 = icmp sgt i32 %4, 5
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
  %13 = load i32, i32* %j
  call void @__print_int(i32 %13)
  br label %if_end

if_else:                                          ; preds = %loop_body
  call void @__print(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @0, i32 0, i32 0))
  br label %if_end

if_end:                                           ; preds = %if_else, %if_then
  br label %loop_test
}

declare void @__print_int(i32)

declare void @__print(i8*)
