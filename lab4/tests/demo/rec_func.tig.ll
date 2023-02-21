; ModuleID = 'tiger'
source_filename = "tiger"

define i32 @main() {
entry:
  br label %body

body:                                             ; preds = %entry
  %call = call i32 @main.fibonacci(i32 5)
  call void @__print_int(i32 %call)
  ret i32 0
}

define internal i32 @main.fibonacci(i32 %n) {
entry:
  %n1 = alloca i32
  %if_result = alloca i32
  %if_result8 = alloca i32
  %if_result9 = alloca i32
  br label %body

body:                                             ; preds = %entry
  store i32 %n, i32* %n1
  %0 = load i32, i32* %n1
  br i1 false, label %if_then2, label %if_else3

if_then:                                          ; preds = %if_end4
  store i32 1, i32* %if_result9
  br label %if_end

if_else:                                          ; preds = %if_end4
  %1 = load i32, i32* %n1
  %2 = sub i32 %1, 1
  %call = call i32 @main.fibonacci(i32 %2)
  %3 = load i32, i32* %n1
  %4 = sub i32 %3, 2
  %call10 = call i32 @main.fibonacci(i32 %4)
  %5 = add i32 %call, %call10
  store i32 %5, i32* %if_result9
  br label %if_end

if_end:                                           ; preds = %if_else, %if_then
  %6 = load i32, i32* %if_result9
  ret i32 %6

if_then2:                                         ; preds = %body
  store i32 1, i32* %if_result
  br label %if_end4

if_else3:                                         ; preds = %body
  %7 = load i32, i32* %n1
  %8 = icmp ne i32 %7, 0
  br i1 %8, label %if_then5, label %if_else6

if_end4:                                          ; preds = %if_end7, %if_then2
  %9 = load i32, i32* %if_result
  %10 = icmp eq i32 %0, %9
  %11 = zext i1 %10 to i32
  %12 = icmp eq i32 %11, 1
  %13 = zext i1 %12 to i32
  %14 = icmp ne i32 %13, 0
  br i1 %14, label %if_then, label %if_else

if_then5:                                         ; preds = %if_else3
  store i32 1, i32* %if_result8
  br label %if_end7

if_else6:                                         ; preds = %if_else3
  store i32 0, i32* %if_result8
  br label %if_end7

if_end7:                                          ; preds = %if_else6, %if_then5
  %15 = load i32, i32* %if_result8
  store i32 %15, i32* %if_result
  br label %if_end4
}

declare void @__print_int(i32)
