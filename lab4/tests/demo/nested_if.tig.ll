; ModuleID = 'tiger'
source_filename = "tiger"

@0 = private unnamed_addr constant [7 x i8] c"a > 2\0A\00", align 1
@1 = private unnamed_addr constant [7 x i8] c"a > 5\0A\00", align 1

define i32 @main() {
entry:
  %a = alloca i32
  br label %body

body:                                             ; preds = %entry
  store i32 1, i32* %a
  %0 = load i32, i32* %a
  %1 = icmp slt i32 %0, 5
  %2 = zext i1 %1 to i32
  %3 = icmp ne i32 %2, 0
  br i1 %3, label %if_then, label %if_else

if_then:                                          ; preds = %body
  %4 = load i32, i32* %a
  %5 = icmp slt i32 %4, 2
  %6 = zext i1 %5 to i32
  %7 = icmp ne i32 %6, 0
  br i1 %7, label %if_then1, label %if_else2

if_else:                                          ; preds = %body
  call void @__print(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @1, i32 0, i32 0))
  br label %if_end

if_end:                                           ; preds = %if_else, %if_end3
  ret i32 0

if_then1:                                         ; preds = %if_then
  %8 = load i32, i32* %a
  call void @__print_int(i32 %8)
  br label %if_end3

if_else2:                                         ; preds = %if_then
  call void @__print(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @0, i32 0, i32 0))
  br label %if_end3

if_end3:                                          ; preds = %if_else2, %if_then1
  br label %if_end
}

declare void @__print_int(i32)

declare void @__print(i8*)
