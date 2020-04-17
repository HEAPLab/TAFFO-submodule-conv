; ModuleID = 'a.4.magiclangtmp.ll'
source_filename = "simple.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str.1 = private unnamed_addr constant [9 x i8] c"simple.c\00", section "llvm.metadata", !taffo.info !0
@.str.3 = private unnamed_addr constant [1 x i8] zeroinitializer, section "llvm.metadata", !taffo.info !2
@.str.4 = private unnamed_addr constant [3 x i8] c"%f\00", align 1, !taffo.info !4
@.str.5 = private unnamed_addr constant [7 x i8] c"%f, %d\00", align 1, !taffo.info !4
@.str.6 = private unnamed_addr constant [7 x i8] c"%d, %f\00", align 1, !taffo.info !4
@.str.7 = private unnamed_addr constant [3 x i8] c"%d\00", align 1, !taffo.info !6

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @main() #0 !taffo.initweight !10 !taffo.funinfo !10 {
entry:
  %a.1flp = alloca float, align 8, !taffo.info !11
  %b.1flp = alloca float, align 8, !taffo.info !14
  %c = alloca double, align 8
  %c3 = bitcast double* %c to i8*
  call void @llvm.var.annotation(i8* %c3, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str.3, i32 0, i32 0), i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str.1, i32 0, i32 0), i32 7), !taffo.constinfo !17
  %0 = bitcast float* %a.1flp to double*, !taffo.info !11
  %call.flt = call i32 (i8*, ...) @__isoc99_scanf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.4, i32 0, i32 0), double* %0), !taffo.info !18, !taffo.initweight !19, !taffo.constinfo !20
  %1 = bitcast float* %b.1flp to double*, !taffo.info !14
  %call4.flt = call i32 (i8*, ...) @__isoc99_scanf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.4, i32 0, i32 0), double* %1), !taffo.info !21, !taffo.initweight !19, !taffo.constinfo !20
  %"1flp3" = load float, float* %a.1flp, align 8, !taffo.info !11
  %"1flp6" = load float, float* %b.1flp, align 8, !taffo.info !14
  %mul.1flp = fmul float %"1flp3", %"1flp6", !taffo.info !11
  %"1flp2" = load float, float* %a.1flp, align 8, !taffo.info !11
  %div.1flp = fdiv float %mul.1flp, %"1flp2", !taffo.info !11
  %2 = fpext float %div.1flp to double, !taffo.info !11
  %call5.flt = call double @exp(double %2) #1, !taffo.info !11, !taffo.initweight !22, !taffo.constinfo !23
  %call5.flt.fallback.1flp = fptrunc double %call5.flt to float, !taffo.info !11
  %3 = fpext float %call5.flt.fallback.1flp to double, !taffo.info !11
  store double %3, double* %c, align 8, !taffo.info !24
  %"1flp1" = load float, float* %a.1flp, align 8, !taffo.info !11
  %"1flp5" = load float, float* %b.1flp, align 8, !taffo.info !14
  %4 = fcmp oeq float %"1flp1", %"1flp5", !taffo.info !18
  br i1 %4, label %if.then, label %if.else, !taffo.info !24, !taffo.initweight !22

if.then:                                          ; preds = %entry
  %5 = load double, double* %c, align 8
  %conv = fptosi double %5 to i32
  %6 = load double, double* %c, align 8
  %call6 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str.5, i32 0, i32 0), double %6, i32 2), !taffo.constinfo !25
  br label %if.end

if.else:                                          ; preds = %entry
  %"1flp" = load float, float* %a.1flp, align 8, !taffo.info !11
  %7 = fmul float 1.000000e+00, %"1flp", !taffo.info !11
  %8 = fptosi float %7 to i32, !taffo.info !18
  %9 = load double, double* %c, align 8
  %call8 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str.6, i32 0, i32 0), i32 3, double %9), !taffo.constinfo !25
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %d.0 = phi i32 [ %conv, %if.then ], [ %8, %if.else ]
  %call9 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.7, i32 0, i32 0), i32 %d.0), !taffo.constinfo !20
  %"1flp4" = load float, float* %b.1flp, align 8, !taffo.info !14
  %conv10 = sitofp i32 %d.0 to double
  %call11.1flp = call float @calledSum.1_1flp(float %"1flp4", double %conv10), !taffo.info !14, !taffo.constinfo !20
  %10 = fpext float %call11.1flp to double, !taffo.info !14
  %call12.flt = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.7, i32 0, i32 0), double %10), !taffo.info !21, !taffo.initweight !22, !taffo.constinfo !20
  ret i32 0
}

; Function Attrs: nounwind
declare !taffo.initweight !26 !taffo.funinfo !27 void @llvm.var.annotation(i8*, i8*, i8*, i32) #1

declare !taffo.initweight !28 !taffo.funinfo !29 dso_local i32 @__isoc99_scanf(i8*, ...) #2

; Function Attrs: nounwind
declare !taffo.initweight !28 !taffo.funinfo !29 dso_local double @exp(double) #3

declare !taffo.initweight !28 !taffo.funinfo !29 dso_local i32 @printf(i8*, ...) #2

; Function Attrs: noinline nounwind uwtable
define dso_local double @calledSum(double %a, double %b) #0 !taffo.initweight !30 !taffo.funinfo !31 !taffo.equivalentChild !32 {
entry:
  %add = fadd double %a, %b
  ret double %add
}

; Function Attrs: noinline nounwind uwtable
define internal float @calledSum.1_1flp(float %a.1flp, double %b) #0 !taffo.initweight !33 !taffo.funinfo !34 !taffo.sourceFunction !35 {
entry:
  %0 = fpext float %a.1flp to double, !taffo.info !14
  %add = fadd double %0, %b
  %1 = fptrunc double %add to float
  ret float %1
}

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!8}
!llvm.ident = !{!9}

!0 = !{i1 false, !1, i1 false, i2 0}
!1 = !{double 0.000000e+00, double 1.150000e+02}
!2 = !{i1 false, !3, i1 false, i2 0}
!3 = !{double 0.000000e+00, double 0.000000e+00}
!4 = !{i1 false, !5, i1 false, i2 0}
!5 = !{double 0.000000e+00, double 1.020000e+02}
!6 = !{i1 false, !7, i1 false, i2 0}
!7 = !{double 0.000000e+00, double 1.000000e+02}
!8 = !{i32 1, !"wchar_size", i32 4}
!9 = !{!"clang version 8.0.1 (tags/RELEASE_801/final)"}
!10 = !{}
!11 = !{!12, !13, i1 false, i2 1}
!12 = !{!"float", i32 1, double 2.500000e+01}
!13 = !{double 1.000000e+00, double 2.500000e+01}
!14 = !{!15, !16, i1 false, i2 1}
!15 = !{!"float", i32 1, double 2.000000e+00}
!16 = !{double 1.000000e+00, double 2.000000e+00}
!17 = !{i1 false, i1 false, i1 false, i1 false, i1 false}
!18 = !{!12, i1 false, i1 false, i2 1}
!19 = !{i32 1}
!20 = !{i1 false, i1 false, i1 false}
!21 = !{!15, i1 false, i1 false, i2 1}
!22 = !{i32 3}
!23 = !{i1 false, i1 false}
!24 = !{i1 false, !13, i1 false, i2 1}
!25 = !{i1 false, i1 false, i1 false, i1 false}
!26 = !{i32 -1, i32 -1, i32 -1, i32 -1}
!27 = !{i32 0, i1 false, i32 0, i1 false, i32 0, i1 false, i32 0, i1 false}
!28 = !{i32 -1}
!29 = !{i32 0, i1 false}
!30 = !{i32 -1, i32 -1}
!31 = !{i32 0, i1 false, i32 0, i1 false}
!32 = distinct !{null}
!33 = !{i32 2, i32 -1}
!34 = !{i32 1, !14, i32 0, i1 false}
!35 = !{double (double, double)* @calledSum}
