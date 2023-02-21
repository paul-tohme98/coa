; ModuleID = 'tiger'
source_filename = "tiger"

define i32 @main() {
entry:
  %x = alloca i32
  br label %body

body:                                             ; preds = %entry
  store i32 15, i32* %x
  %0 = load i32, i32* %x
  %call = call i32 @main.getTemperature(i32 %0)
  ret i32 0
}

define internal i32 @main.getTemperature(i32 %temp) {
entry:
  %temp1 = alloca i32
  %if_result = alloca i32
  br label %body

body:                                             ; preds = %entry
  store i32 %temp, i32* %temp1
  %0 = load i32, i32* %temp1
  %1 = icmp slt i32 %0, 5
  %2 = zext i1 %1 to i32
  %3 = icmp ne i32 %2, 0
  br i1 %3, label %if_then, label %if_else

if_then:                                          ; preds = %body
  store i32 1, i32* %if_result
  br label %if_end

if_else:                                          ; preds = %body
  store i32 0, i32* %if_result
  br label %if_end

if_end:                                           ; preds = %if_else, %if_then
  %4 = load i32, i32* %if_result
  ret i32 %4
}
