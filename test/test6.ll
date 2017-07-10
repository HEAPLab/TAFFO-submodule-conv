; ModuleID = 'test6.c'
source_filename = "test6.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

@.str = private unnamed_addr constant [9 x i8] c"no_float\00", section "llvm.metadata"
@.str.1 = private unnamed_addr constant [8 x i8] c"test6.c\00", section "llvm.metadata"

; Function Attrs: noinline nounwind ssp uwtable
define float @test(i32 %a) #0 {
entry:
  %a.addr = alloca i32, align 4
  %c = alloca [10 x float], align 16
  %b = alloca float*, align 8
  store i32 %a, i32* %a.addr, align 4
  %c1 = bitcast [10 x float]* %c to i8*
  call void @llvm.var.annotation(i8* %c1, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.1, i32 0, i32 0), i32 4)
  %b2 = bitcast float** %b to i8*
  call void @llvm.var.annotation(i8* %b2, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.1, i32 0, i32 0), i32 5)
  %arraydecay = getelementptr inbounds [10 x float], [10 x float]* %c, i32 0, i32 0
  store float* %arraydecay, float** %b, align 8
  %0 = load i32, i32* %a.addr, align 4
  %conv = sitofp i32 %0 to float
  %1 = load float*, float** %b, align 8
  %arrayidx = getelementptr inbounds float, float* %1, i64 5
  store float %conv, float* %arrayidx, align 4
  %2 = load float*, float** %b, align 8
  %arrayidx3 = getelementptr inbounds float, float* %2, i64 5
  %3 = load float, float* %arrayidx3, align 4
  ret float %3
}

; Function Attrs: nounwind
declare void @llvm.var.annotation(i8*, i8*, i8*, i32) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"PIC Level", i32 2}
!1 = !{!"clang version 4.0.0 (tags/RELEASE_400/final)"}
