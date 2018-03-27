; ModuleID = 'test9.c'
source_filename = "test9.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.13.0"

@.str = private unnamed_addr constant [9 x i8] c"no_float\00", section "llvm.metadata"
@.str.1 = private unnamed_addr constant [8 x i8] c"test9.c\00", section "llvm.metadata"

; Function Attrs: noinline nounwind ssp uwtable
define float @oven(i32 %stuff, i32 %baked, float %cherry) #0 {
entry:
  %stuff.addr = alloca i32, align 4
  %baked.addr = alloca i32, align 4
  %cherry.addr = alloca float, align 4
  %cake = alloca float, align 4
  store i32 %stuff, i32* %stuff.addr, align 4
  store i32 %baked, i32* %baked.addr, align 4
  store float %cherry, float* %cherry.addr, align 4
  %cake1 = bitcast float* %cake to i8*
  call void @llvm.var.annotation(i8* %cake1, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.1, i32 0, i32 0), i32 4)
  %0 = load i32, i32* %baked.addr, align 4
  %1 = load i32, i32* %stuff.addr, align 4
  %add = add nsw i32 %0, %1
  %conv = sitofp i32 %add to float
  store float %conv, float* %cake, align 4
  %2 = load float, float* %cake, align 4
  %3 = load float, float* %cherry.addr, align 4
  %add2 = fadd float %2, %3
  ret float %add2
}

; Function Attrs: nounwind
declare void @llvm.var.annotation(i8*, i8*, i8*, i32) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"PIC Level", i32 2}
!1 = !{!"clang version 4.0.0 (tags/RELEASE_400/final)"}
