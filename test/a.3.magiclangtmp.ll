; ModuleID = 'a.2.magiclangtmp.ll'
source_filename = "simple.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [20 x i8] c"scalar(range(1,25))\00", section "llvm.metadata", !taffo.info !0
@.str.1 = private unnamed_addr constant [9 x i8] c"simple.c\00", section "llvm.metadata", !taffo.info !0
@.str.2 = private unnamed_addr constant [19 x i8] c"scalar(range(1,2))\00", section "llvm.metadata", !taffo.info !0
@.str.3 = private unnamed_addr constant [1 x i8] zeroinitializer, section "llvm.metadata", !taffo.info !2
@.str.4 = private unnamed_addr constant [3 x i8] c"%f\00", align 1, !taffo.info !4
@.str.5 = private unnamed_addr constant [7 x i8] c"%f, %d\00", align 1, !taffo.info !4
@.str.6 = private unnamed_addr constant [7 x i8] c"%d, %f\00", align 1, !taffo.info !4
@.str.7 = private unnamed_addr constant [3 x i8] c"%d\00", align 1, !taffo.info !6

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @main() #0 !taffo.initweight !10 !taffo.funinfo !10 {
entry:
  %a = alloca double, align 8, !taffo.initweight !11, !taffo.info !12
  %b = alloca double, align 8, !taffo.initweight !11, !taffo.info !14
  %c = alloca double, align 8
  %a1 = bitcast double* %a to i8*, !taffo.initweight !16, !taffo.info !12
  %b2 = bitcast double* %b to i8*, !taffo.initweight !16, !taffo.info !14
  %c3 = bitcast double* %c to i8*
  call void @llvm.var.annotation(i8* %c3, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str.3, i32 0, i32 0), i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str.1, i32 0, i32 0), i32 7), !taffo.constinfo !17
  %call = call i32 (i8*, ...) @__isoc99_scanf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.4, i32 0, i32 0), double* %a), !taffo.initweight !16, !taffo.info !12, !taffo.constinfo !18
  %call4 = call i32 (i8*, ...) @__isoc99_scanf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.4, i32 0, i32 0), double* %b), !taffo.initweight !16, !taffo.info !14, !taffo.constinfo !18
  %0 = load double, double* %a, align 8, !taffo.initweight !16, !taffo.info !12
  %1 = load double, double* %b, align 8, !taffo.initweight !16, !taffo.info !14
  %mul = fmul double %0, %1, !taffo.initweight !19, !taffo.info !12
  %2 = load double, double* %a, align 8, !taffo.initweight !16, !taffo.info !12
  %div = fdiv double %mul, %2, !taffo.initweight !19, !taffo.info !12
  %call5 = call double @exp(double %div) #1, !taffo.initweight !20, !taffo.info !12, !taffo.constinfo !21
  store double %call5, double* %c, align 8, !taffo.initweight !22, !taffo.info !12
  %3 = load double, double* %a, align 8, !taffo.initweight !16, !taffo.info !12
  %4 = load double, double* %b, align 8, !taffo.initweight !16, !taffo.info !14
  %cmp = fcmp oeq double %3, %4, !taffo.initweight !19, !taffo.info !12
  br i1 %cmp, label %if.then, label %if.else, !taffo.initweight !20, !taffo.info !12

if.then:                                          ; preds = %entry
  %5 = load double, double* %c, align 8
  %conv = fptosi double %5 to i32
  %6 = load double, double* %c, align 8
  %call6 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str.5, i32 0, i32 0), double %6, i32 2), !taffo.constinfo !23
  br label %if.end

if.else:                                          ; preds = %entry
  %7 = load double, double* %a, align 8, !taffo.initweight !16, !taffo.info !12
  %conv7 = fptosi double %7 to i32, !taffo.initweight !19, !taffo.info !12
  %8 = load double, double* %c, align 8
  %call8 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str.6, i32 0, i32 0), i32 3, double %8), !taffo.constinfo !23
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %d.0 = phi i32 [ %conv, %if.then ], [ %conv7, %if.else ]
  %call9 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.7, i32 0, i32 0), i32 %d.0), !taffo.constinfo !18
  %9 = load double, double* %b, align 8, !taffo.initweight !16, !taffo.info !14
  %conv10 = sitofp i32 %d.0 to double
  %call11 = call double @calledSum.1(double %9, double %conv10), !taffo.initweight !19, !taffo.info !14, !taffo.originalCall !24, !taffo.constinfo !18
  %call12 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.7, i32 0, i32 0), double %call11), !taffo.initweight !20, !taffo.info !14, !taffo.constinfo !18
  ret i32 0
}

; Function Attrs: nounwind
declare !taffo.initweight !25 !taffo.funinfo !26 void @llvm.var.annotation(i8*, i8*, i8*, i32) #1

declare !taffo.initweight !27 !taffo.funinfo !28 dso_local i32 @__isoc99_scanf(i8*, ...) #2

; Function Attrs: nounwind
declare !taffo.initweight !27 !taffo.funinfo !28 dso_local double @exp(double) #3

declare !taffo.initweight !27 !taffo.funinfo !28 dso_local i32 @printf(i8*, ...) #2

; Function Attrs: noinline nounwind uwtable
define dso_local double @calledSum(double %a, double %b) #0 !taffo.initweight !29 !taffo.funinfo !30 !taffo.equivalentChild !31 {
entry:
  %add = fadd double %a, %b
  ret double %add
}

; Function Attrs: noinline nounwind uwtable
define internal double @calledSum.1(double %a, double %b) #0 !taffo.initweight !32 !taffo.funinfo !33 !taffo.sourceFunction !24 {
entry:
  %add = fadd double %a, %b
  ret double %add
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
!11 = !{i32 0}
!12 = !{i1 false, !13, i1 false, i2 1}
!13 = !{double 1.000000e+00, double 2.500000e+01}
!14 = !{i1 false, !15, i1 false, i2 1}
!15 = !{double 1.000000e+00, double 2.000000e+00}
!16 = !{i32 1}
!17 = !{i1 false, i1 false, i1 false, i1 false, i1 false}
!18 = !{i1 false, i1 false, i1 false}
!19 = !{i32 2}
!20 = !{i32 3}
!21 = !{i1 false, i1 false}
!22 = !{i32 4}
!23 = !{i1 false, i1 false, i1 false, i1 false}
!24 = !{double (double, double)* @calledSum}
!25 = !{i32 -1, i32 -1, i32 -1, i32 -1}
!26 = !{i32 0, i1 false, i32 0, i1 false, i32 0, i1 false, i32 0, i1 false}
!27 = !{i32 -1}
!28 = !{i32 0, i1 false}
!29 = !{i32 -1, i32 -1}
!30 = !{i32 0, i1 false, i32 0, i1 false}
!31 = !{double (double, double)* @calledSum.1}
!32 = !{i32 2, i32 -1}
!33 = !{i32 1, !14, i32 0, i1 false}
