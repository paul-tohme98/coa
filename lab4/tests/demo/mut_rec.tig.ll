; ModuleID = 'tiger'
source_filename = "tiger"

define i32 @main() {
entry:
  br label %body

body:                                             ; preds = %entry
  %call = call i32 @main.is_even(i32 10)
  call void @__print_int(i32 %call)
  ret i32 0
}

define internal i32 @main.is_even(i32 %n) {
entry:
  %n1 = alloca i32
  %if_result = alloca i32
  br label %body

body:                                             ; preds = %entry
  store i32 %n, i32* %n1
  %0 = load i32, i32* %n1
  %1 = icmp eq i32 %0, 0
  %2 = zext i1 %1 to i32
  %3 = icmp ne i32 %2, 0
  br i1 %3, label %if_then, label %if_else

if_then:                                          ; preds = %body
  store i32 1, i32* %if_result
  br label %if_end

if_else:                                          ; preds = %body
  %4 = load i32, i32* %n1
  %5 = sub i32 %4, 1
  %call = call i32 @main.is_odd(i32 %5)
  store i32 %call, i32* %if_result
  br label %if_end

if_end:                                           ; preds = %if_else, %if_then
  %6 = load i32, i32* %if_result
  ret i32 %6
}

define internal i32 @main.is_odd(i32 %n) {
entry:
  %n1 = alloca i32
  %if_result = alloca i32
  br label %body

body:                                             ; preds = %entry
  store i32 %n, i32* %n1
  %0 = load i32, i32* %n1
  %1 = icmp eq i32 %0, 0
  %2 = zext i1 %1 to i32
  %3 = icmp ne i32 %2, 0
  br i1 %3, label %if_then, label %if_else

if_then:                                          ; preds = %body
  store i32 0, i32* %if_result
  br label %if_end

if_else:                                          ; preds = %body
  %4 = load i32, i32* %n1
  %5 = sub i32 %4, 1
  %call = call i32 @main.is_even(i32 %5)
  store i32 %call, i32* %if_result
  br label %if_end

if_end:                                           ; preds = %if_else, %if_then
  %6 = load i32, i32* %if_result
  ret i32 %6
}

declare void @__print_int(i32)
