; ModuleID = 'a.2.magiclangtmp.ll'
source_filename = "simple2.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [32 x i8] c"target('x') scalar(range(1,25))\00", section "llvm.metadata", !taffo.info !0
@.str.1 = private unnamed_addr constant [10 x i8] c"simple2.c\00", section "llvm.metadata", !taffo.info !2
@.str.2 = private unnamed_addr constant [32 x i8] c"target('y') scalar(range(7,14))\00", section "llvm.metadata", !taffo.info !4
@.str.3 = private unnamed_addr constant [4 x i8] c"%lf\00", align 1, !taffo.info !6
@result = common dso_local global float 0.000000e+00, align 4, !taffo.info !8
@result_ptr = common dso_local global float* null, align 8, !taffo.info !8

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @main() #0 !taffo.start !12 !taffo.initweight !13 !taffo.funinfo !13 {
entry:
  %x = alloca float, align 4, !taffo.initweight !14, !taffo.target !15, !taffo.info !16
  %y = alloca float, align 4, !taffo.initweight !14, !taffo.target !18, !taffo.info !19
  %x1 = bitcast float* %x to i8*, !taffo.initweight !21, !taffo.target !15, !taffo.info !16
  %y2 = bitcast float* %y to i8*, !taffo.initweight !21, !taffo.target !18, !taffo.info !19
  %call = call i32 (i8*, ...) @__isoc99_scanf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str.3, i32 0, i32 0), float* %x), !taffo.initweight !21, !taffo.target !15, !taffo.info !16, !taffo.constinfo !22
  %call3 = call i32 (i8*, ...) @__isoc99_scanf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str.3, i32 0, i32 0), float* %y), !taffo.initweight !21, !taffo.target !18, !taffo.info !19, !taffo.constinfo !22
  store float* @result, float** @result_ptr, align 8, !taffo.constinfo !23
  %0 = load float, float* %x, align 4, !taffo.initweight !21, !taffo.target !15, !taffo.info !16
  %1 = load float, float* %y, align 4, !taffo.initweight !21, !taffo.target !18, !taffo.info !19
  %add = fadd float %0, %1, !taffo.initweight !24, !taffo.target !15, !taffo.info !25
  %2 = load float*, float** @result_ptr, align 8, !taffo.info !8
  store float %add, float* %2, align 4, !taffo.initweight !27, !taffo.target !15, !taffo.info !16
  %3 = load float*, float** @result_ptr, align 8, !taffo.info !8
  %4 = load float, float* %3, align 4, !taffo.info !8
  %conv = fpext float %4 to double, !taffo.info !8
  %call4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str.3, i32 0, i32 0), double %conv), !taffo.constinfo !22
  ret i32 0, !taffo.info !28
}

; Function Attrs: nounwind
declare !taffo.initweight !30 !taffo.funinfo !31 void @llvm.var.annotation(i8*, i8*, i8*, i32) #1

declare !taffo.initweight !32 !taffo.funinfo !33 dso_local i32 @__isoc99_scanf(i8*, ...) #2

declare !taffo.initweight !32 !taffo.funinfo !33 dso_local i32 @printf(i8*, ...) #2

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!10}
!llvm.ident = !{!11}

!0 = !{i1 false, !1, i1 false, i2 0}
!1 = !{double 0.000000e+00, double 1.200000e+02}
!2 = !{i1 false, !3, i1 false, i2 0}
!3 = !{double 0.000000e+00, double 1.150000e+02}
!4 = !{i1 false, !5, i1 false, i2 0}
!5 = !{double 0.000000e+00, double 1.210000e+02}
!6 = !{i1 false, !7, i1 false, i2 0}
!7 = !{double 0.000000e+00, double 1.080000e+02}
!8 = !{i1 false, !9, i1 false, i2 0}
!9 = !{double 0.000000e+00, double 3.900000e+01}
!10 = !{i32 1, !"wchar_size", i32 4}
!11 = !{!"clang version 8.0.1 (tags/RELEASE_801/final)"}
!12 = !{i1 true}
!13 = !{}
!14 = !{i32 0}
!15 = !{!"x"}
!16 = !{i1 false, !17, i1 false, i2 1}
!17 = !{double 1.000000e+00, double 2.500000e+01}
!18 = !{!"y"}
!19 = !{i1 false, !20, i1 false, i2 1}
!20 = !{double 7.000000e+00, double 1.400000e+01}
!21 = !{i32 1}
!22 = !{i1 false, i1 false, i1 false}
!23 = !{i1 false, i1 false}
!24 = !{i32 2}
!25 = !{i1 false, !26, i1 false, i2 1}
!26 = !{double 8.000000e+00, double 3.900000e+01}
!27 = !{i32 3}
!28 = !{i1 false, !29, i1 false, i2 0}
!29 = !{double 0.000000e+00, double 0.000000e+00}
!30 = !{i32 -1, i32 -1, i32 -1, i32 -1}
!31 = !{i32 0, i1 false, i32 0, i1 false, i32 0, i1 false, i32 0, i1 false}
!32 = !{i32 -1}
!33 = !{i32 0, i1 false}
