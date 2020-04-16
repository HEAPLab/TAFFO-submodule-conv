; ModuleID = 'a.4.magiclangtmp.ll'
source_filename = "simple2.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str.3 = private unnamed_addr constant [4 x i8] c"%lf\00", align 1, !taffo.info !0
@result = common dso_local global float 0.000000e+00, align 4, !taffo.info !2
@result_ptr = common dso_local global float* null, align 8, !taffo.info !2

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @main() #0 !taffo.start !6 !taffo.initweight !7 !taffo.funinfo !7 {
entry:
  %x = alloca float, align 4, !taffo.info !8, !taffo.initweight !11, !taffo.target !12
  %y = alloca float, align 4, !taffo.info !13, !taffo.initweight !11, !taffo.target !16
  %call = call i32 (i8*, ...) @__isoc99_scanf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str.3, i32 0, i32 0), float* %x), !taffo.info !17, !taffo.initweight !18, !taffo.target !12, !taffo.constinfo !19
  %call3 = call i32 (i8*, ...) @__isoc99_scanf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str.3, i32 0, i32 0), float* %y), !taffo.info !20, !taffo.initweight !18, !taffo.target !16, !taffo.constinfo !19
  store float* @result, float** @result_ptr, align 8, !taffo.constinfo !21
  %0 = load float, float* %x, align 4, !taffo.info !22, !taffo.initweight !18, !taffo.target !12
  %1 = load float, float* %y, align 4, !taffo.info !24, !taffo.initweight !18, !taffo.target !16
  %add = fadd float %0, %1, !taffo.info !25, !taffo.initweight !27, !taffo.target !12
  %2 = load float*, float** @result_ptr, align 8, !taffo.info !2
  store float %add, float* %2, align 4, !taffo.info !28, !taffo.target !12
  %3 = load float*, float** @result_ptr, align 8, !taffo.info !2
  %4 = load float, float* %3, align 4, !taffo.info !2
  %conv = fpext float %4 to double, !taffo.info !2
  %call4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str.3, i32 0, i32 0), double %conv), !taffo.constinfo !19
  ret i32 0, !taffo.info !29
}

declare !taffo.initweight !31 !taffo.funinfo !32 dso_local i32 @__isoc99_scanf(i8*, ...) #1

declare !taffo.initweight !31 !taffo.funinfo !32 dso_local i32 @printf(i8*, ...) #1

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!4}
!llvm.ident = !{!5}

!0 = !{i1 false, !1, i1 false, i2 0}
!1 = !{double 0.000000e+00, double 1.080000e+02}
!2 = !{i1 false, !3, i1 false, i2 0}
!3 = !{double 0.000000e+00, double 3.900000e+01}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{!"clang version 8.0.1 (tags/RELEASE_801/final)"}
!6 = !{i1 true}
!7 = !{}
!8 = !{!9, !10, i1 false, i2 1}
!9 = !{!"fixp", i32 32, i32 27}
!10 = !{double 1.000000e+00, double 2.500000e+01}
!11 = !{i32 0}
!12 = !{!"x"}
!13 = !{!14, !15, i1 false, i2 1}
!14 = !{!"fixp", i32 32, i32 28}
!15 = !{double 7.000000e+00, double 1.400000e+01}
!16 = !{!"y"}
!17 = !{!9, i1 false, i1 false, i2 1}
!18 = !{i32 1}
!19 = !{i1 false, i1 false, i1 false}
!20 = !{!14, i1 false, i1 false, i2 1}
!21 = !{i1 false, i1 false}
!22 = !{!23, !10, i1 false, i2 1}
!23 = !{!"fixp", i32 32, i32 26}
!24 = !{!23, !15, i1 false, i2 1}
!25 = !{!23, !26, i1 false, i2 1}
!26 = !{double 8.000000e+00, double 3.900000e+01}
!27 = !{i32 2}
!28 = !{i1 false, !10, i1 false, i2 1}
!29 = !{i1 false, !30, i1 false, i2 0}
!30 = !{double 0.000000e+00, double 0.000000e+00}
!31 = !{i32 -1}
!32 = !{i32 0, i1 false}
