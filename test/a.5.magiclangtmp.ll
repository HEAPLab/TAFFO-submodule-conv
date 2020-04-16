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
  %x.u5_27fixp = alloca i32, align 4, !taffo.info !8, !taffo.target !11
  %y.u4_28fixp = alloca i32, align 4, !taffo.info !12, !taffo.target !15
  %0 = bitcast i32* %x.u5_27fixp to float*, !taffo.info !8, !taffo.target !11
  %call.flt = call i32 (i8*, ...) @__isoc99_scanf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str.3, i32 0, i32 0), float* %0), !taffo.info !16, !taffo.initweight !17, !taffo.target !11, !taffo.constinfo !18
  %1 = bitcast i32* %y.u4_28fixp to float*, !taffo.info !12, !taffo.target !15
  %call3.flt = call i32 (i8*, ...) @__isoc99_scanf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str.3, i32 0, i32 0), float* %1), !taffo.info !19, !taffo.initweight !17, !taffo.target !15, !taffo.constinfo !18
  store float* @result, float** @result_ptr, align 8, !taffo.constinfo !20
  %u5_27fixp = load i32, i32* %x.u5_27fixp, align 4, !taffo.info !21, !taffo.target !11
  %u4_28fixp = load i32, i32* %y.u4_28fixp, align 4, !taffo.info !23, !taffo.target !15
  %2 = lshr i32 %u5_27fixp, 1, !taffo.info !21, !taffo.target !11
  %3 = lshr i32 %u4_28fixp, 2, !taffo.info !23, !taffo.target !15
  %add.u6_26fixp = add i32 %2, %3, !taffo.info !24, !taffo.target !11
  %4 = uitofp i32 %add.u6_26fixp to float, !taffo.info !24, !taffo.target !11
  %5 = fdiv float %4, 0x4190000000000000, !taffo.info !24, !taffo.target !11
  %6 = load float*, float** @result_ptr, align 8, !taffo.info !2
  store float %5, float* %6, align 4, !taffo.info !26, !taffo.target !11
  %7 = load float*, float** @result_ptr, align 8, !taffo.info !2
  %8 = load float, float* %7, align 4, !taffo.info !2
  %conv = fpext float %8 to double, !taffo.info !2
  %call4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str.3, i32 0, i32 0), double %conv), !taffo.constinfo !18
  ret i32 0, !taffo.info !27
}

declare !taffo.initweight !29 !taffo.funinfo !30 dso_local i32 @__isoc99_scanf(i8*, ...) #1

declare !taffo.initweight !29 !taffo.funinfo !30 dso_local i32 @printf(i8*, ...) #1

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
!11 = !{!"x"}
!12 = !{!13, !14, i1 false, i2 1}
!13 = !{!"fixp", i32 32, i32 28}
!14 = !{double 7.000000e+00, double 1.400000e+01}
!15 = !{!"y"}
!16 = !{!9, i1 false, i1 false, i2 1}
!17 = !{i32 1}
!18 = !{i1 false, i1 false, i1 false}
!19 = !{!13, i1 false, i1 false, i2 1}
!20 = !{i1 false, i1 false}
!21 = !{!22, !10, i1 false, i2 1}
!22 = !{!"fixp", i32 32, i32 26}
!23 = !{!22, !14, i1 false, i2 1}
!24 = !{!22, !25, i1 false, i2 1}
!25 = !{double 8.000000e+00, double 3.900000e+01}
!26 = !{i1 false, !10, i1 false, i2 1}
!27 = !{i1 false, !28, i1 false, i2 0}
!28 = !{double 0.000000e+00, double 0.000000e+00}
!29 = !{i32 -1}
!30 = !{i32 0, i1 false}
