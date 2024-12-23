; ModuleID = '../test.c'
source_filename = "../test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i8:8:32-i16:16:32-i64:64-i128:128-n32:64-S128-Fn32"
target triple = "aarch64"

@.str = private unnamed_addr constant [13 x i8] c"Hello, world\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca ptr, align 8
  %3 = alloca i32, align 4
  store i32 0, ptr %1, align 4
  store ptr @.str, ptr %2, align 8
  store i32 1, ptr %3, align 4
  br label %4

4:                                                ; preds = %8, %0
  %5 = load i32, ptr %3, align 4
  %6 = icmp sle i32 %5, 6969
  br i1 %6, label %7, label %11

7:                                                ; preds = %4
  call void @inspect(ptr noundef %3)
  br label %8

8:                                                ; preds = %7
  %9 = load i32, ptr %3, align 4
  %10 = add nsw i32 %9, 1
  store i32 %10, ptr %3, align 4
  br label %4, !llvm.loop !4

11:                                               ; preds = %4
  ret i32 0
}

declare dso_local void @inspect(ptr noundef) #1

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fp-armv8,+neon,+v8a,-fmv" }
attributes #1 = { "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fp-armv8,+neon,+v8a,-fmv" }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{i32 7, !"frame-pointer", i32 1}
!3 = !{!"clang version 20.0.0git (https://github.com/llvm/llvm-project.git e072cffe3b639e8c433138b10ff68dc577497cf8)"}
!4 = distinct !{!4, !5}
!5 = !{!"llvm.loop.mustprogress"}
