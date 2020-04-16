; ModuleID = 'a.1.magiclangtmp.ll'
source_filename = "simple2.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [32 x i8] c"target('x') scalar(range(1,25))\00", section "llvm.metadata"
@.str.1 = private unnamed_addr constant [10 x i8] c"simple2.c\00", section "llvm.metadata"
@.str.2 = private unnamed_addr constant [32 x i8] c"target('y') scalar(range(7,14))\00", section "llvm.metadata"
@.str.3 = private unnamed_addr constant [4 x i8] c"%lf\00", align 1
@result = common dso_local global float 0.000000e+00, align 4
@result_ptr = common dso_local global float* null, align 8

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @main() #0 !taffo.start !2 !taffo.initweight !3 !taffo.funinfo !3 {
entry:
  %retval = alloca i32, align 4
  %x = alloca float, align 4, !taffo.initweight !4, !taffo.target !5, !taffo.info !6
  %y = alloca float, align 4, !taffo.initweight !4, !taffo.target !8, !taffo.info !9
  store i32 0, i32* %retval, align 4
  %x1 = bitcast float* %x to i8*, !taffo.initweight !11, !taffo.target !5, !taffo.info !6
  %y2 = bitcast float* %y to i8*, !taffo.initweight !11, !taffo.target !8, !taffo.info !9
  %call = call i32 (i8*, ...) @__isoc99_scanf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str.3, i32 0, i32 0), float* %x), !taffo.initweight !11, !taffo.target !5, !taffo.info !6
  %call3 = call i32 (i8*, ...) @__isoc99_scanf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str.3, i32 0, i32 0), float* %y), !taffo.initweight !11, !taffo.target !8, !taffo.info !9
  store float* @result, float** @result_ptr, align 8
  %0 = load float, float* %x, align 4, !taffo.initweight !11, !taffo.target !5, !taffo.info !6
  %1 = load float, float* %y, align 4, !taffo.initweight !11, !taffo.target !8, !taffo.info !9
  %add = fadd float %0, %1, !taffo.initweight !12, !taffo.target !5, !taffo.info !6
  %2 = load float*, float** @result_ptr, align 8
  store float %add, float* %2, align 4, !taffo.initweight !13, !taffo.target !5, !taffo.info !6
  %3 = load float*, float** @result_ptr, align 8
  %4 = load float, float* %3, align 4
  %conv = fpext float %4 to double
  %call4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str.3, i32 0, i32 0), double %conv)
  ret i32 0
}

; Function Attrs: nounwind
declare !taffo.initweight !14 !taffo.funinfo !15 void @llvm.var.annotation(i8*, i8*, i8*, i32) #1

declare !taffo.initweight !16 !taffo.funinfo !17 dso_local i32 @__isoc99_scanf(i8*, ...) #2

declare !taffo.initweight !16 !taffo.funinfo !17 dso_local i32 @printf(i8*, ...) #2

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.1 (tags/RELEASE_801/final)"}
!2 = !{i1 true}
!3 = !{}
!4 = !{i32 0}
!5 = !{!"x"}
!6 = !{i1 false, !7, i1 false, i2 1}
!7 = !{double 1.000000e+00, double 2.500000e+01}
!8 = !{!"y"}
!9 = !{i1 false, !10, i1 false, i2 1}
!10 = !{double 7.000000e+00, double 1.400000e+01}
!11 = !{i32 1}
!12 = !{i32 2}
!13 = !{i32 3}
!14 = !{i32 -1, i32 -1, i32 -1, i32 -1}
!15 = !{i32 0, i1 false, i32 0, i1 false, i32 0, i1 false, i32 0, i1 false}
!16 = !{i32 -1}
!17 = !{i32 0, i1 false}
