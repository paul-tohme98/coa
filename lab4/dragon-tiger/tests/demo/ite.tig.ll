; ModuleID = 'tiger'
source_filename = "tiger"

define i32 @main() {
entry:
  %a = alloca i32
  %if_result = alloca i32
  br label %body

body:                                             ; preds = %entry
  br i1 true, label %if_then, label %if_else

if_then:                                          ; preds = %body
  store i32 10, i32* %if_result
  br label %if_end

if_else:                                          ; preds = %body
  store i32 20, i32* %if_result
  br label %if_end

if_end:                                           ; preds = %if_else, %if_then
  %0 = load i32, i32* %if_result
  store i32 %0, i32* %a
  ret i32 0
}
