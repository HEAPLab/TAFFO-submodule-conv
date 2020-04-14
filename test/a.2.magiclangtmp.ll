; ModuleID = 'a.1.magiclangtmp.ll'
source_filename = "simple.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [43 x i8] c"target('a') scalar(range(1,25) error(0.8))\00", section "llvm.metadata"
@.str.1 = private unnamed_addr constant [9 x i8] c"simple.c\00", section "llvm.metadata"
@.str.2 = private unnamed_addr constant [42 x i8] c"target('b') scalar(range(1,2) error(0.9))\00", section "llvm.metadata"
@.str.3 = private unnamed_addr constant [12 x i8] c"target('c')\00", section "llvm.metadata"
@.str.4 = private unnamed_addr constant [3 x i8] c"%f\00", align 1
@.str.5 = private unnamed_addr constant [7 x i8] c"%f, %d\00", align 1
@.str.6 = private unnamed_addr constant [7 x i8] c"%d, %f\00", align 1
@.str.7 = private unnamed_addr constant [3 x i8] c"%d\00", align 1

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @main() #0 !taffo.start !2 !taffo.initweight !3 !taffo.funinfo !3 {
entry:
  %retval = alloca i32, align 4
  %a = alloca float, align 4, !taffo.initweight !4, !taffo.target !5, !taffo.info !6
  %b = alloca float, align 4, !taffo.initweight !4, !taffo.target !9, !taffo.info !10
  %c = alloca float, align 4
  %d = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %a1 = bitcast float* %a to i8*, !taffo.initweight !13, !taffo.target !5, !taffo.info !6
  %b2 = bitcast float* %b to i8*, !taffo.initweight !13, !taffo.target !9, !taffo.info !10
  %c3 = bitcast float* %c to i8*
  call void @llvm.var.annotation(i8* %c3, i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str.3, i32 0, i32 0), i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str.1, i32 0, i32 0), i32 6)
  %call = call i32 (i8*, ...) @__isoc99_scanf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.4, i32 0, i32 0), float* %a), !taffo.initweight !13, !taffo.target !5, !taffo.info !6
  %call4 = call i32 (i8*, ...) @__isoc99_scanf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.4, i32 0, i32 0), float* %b), !taffo.initweight !13, !taffo.target !9, !taffo.info !10
  %0 = load float, float* %a, align 4, !taffo.initweight !13, !taffo.target !5, !taffo.info !6
  %1 = load float, float* %b, align 4, !taffo.initweight !13, !taffo.target !9, !taffo.info !10
  %mul = fmul float %0, %1, !taffo.initweight !14, !taffo.target !5, !taffo.info !6
  %2 = load float, float* %a, align 4, !taffo.initweight !13, !taffo.target !5, !taffo.info !6
  %div = fdiv float %mul, %2, !taffo.initweight !14, !taffo.target !5, !taffo.info !6
  %conv = fpext float %div to double, !taffo.initweight !15, !taffo.target !5, !taffo.info !6
  %call5 = call double @exp(double %conv) #1, !taffo.initweight !16, !taffo.target !5, !taffo.info !6
  %conv6 = fptrunc double %call5 to float, !taffo.initweight !17, !taffo.target !5, !taffo.info !6
  store float %conv6, float* %c, align 4, !taffo.initweight !18, !taffo.target !5, !taffo.info !6
  store i32 0, i32* %d, align 4
  %3 = load float, float* %a, align 4, !taffo.initweight !13, !taffo.target !5, !taffo.info !6
  %4 = load float, float* %b, align 4, !taffo.initweight !13, !taffo.target !9, !taffo.info !10
  %cmp = fcmp oeq float %3, %4, !taffo.initweight !14, !taffo.target !5, !taffo.info !6
  br i1 %cmp, label %if.then, label %if.else, !taffo.initweight !15, !taffo.target !5, !taffo.info !6

if.then:                                          ; preds = %entry
  %5 = load float, float* %c, align 4
  %conv8 = fptosi float %5 to i32
  store i32 %conv8, i32* %d, align 4
  %6 = load float, float* %c, align 4
  %conv9 = fpext float %6 to double
  %call10 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str.5, i32 0, i32 0), double %conv9, i32 2)
  br label %if.end

if.else:                                          ; preds = %entry
  %7 = load float, float* %a, align 4, !taffo.initweight !13, !taffo.target !5, !taffo.info !6
  %conv11 = fptosi float %7 to i32, !taffo.initweight !14, !taffo.target !5, !taffo.info !6
  store i32 %conv11, i32* %d, align 4, !taffo.initweight !15, !taffo.target !5, !taffo.info !6
  %8 = load float, float* %c, align 4
  %conv12 = fpext float %8 to double
  %call13 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str.6, i32 0, i32 0), i32 3, double %conv12)
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %9 = load i32, i32* %d, align 4
  %call14 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.7, i32 0, i32 0), i32 %9)
  %10 = load float, float* %b, align 4, !taffo.initweight !13, !taffo.target !9, !taffo.info !10
  %conv15 = fpext float %10 to double, !taffo.initweight !14, !taffo.target !9, !taffo.info !10
  %11 = load i32, i32* %d, align 4
  %conv16 = sitofp i32 %11 to double
  %call17 = call double @calledSum.1(double %conv15, double %conv16), !taffo.initweight !15, !taffo.target !9, !taffo.info !10, !taffo.originalCall !19
  %call18 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.7, i32 0, i32 0), double %call17), !taffo.initweight !16, !taffo.target !9, !taffo.info !10
  ret i32 0
}

; Function Attrs: nounwind
declare !taffo.initweight !20 !taffo.funinfo !21 void @llvm.var.annotation(i8*, i8*, i8*, i32) #1

declare !taffo.initweight !22 !taffo.funinfo !23 dso_local i32 @__isoc99_scanf(i8*, ...) #2

; Function Attrs: nounwind
declare !taffo.initweight !22 !taffo.funinfo !23 dso_local double @exp(double) #3

declare !taffo.initweight !22 !taffo.funinfo !23 dso_local i32 @printf(i8*, ...) #2

; Function Attrs: noinline nounwind uwtable
define dso_local double @calledSum(double %a, double %b) #0 !taffo.initweight !24 !taffo.equivalentChild !25 !taffo.funinfo !26 {
entry:
  %a.addr = alloca double, align 8
  %b.addr = alloca double, align 8
  store double %a, double* %a.addr, align 8
  store double %b, double* %b.addr, align 8
  %0 = load double, double* %a.addr, align 8
  %1 = load double, double* %b.addr, align 8
  %add = fadd double %0, %1
  ret double %add
}

; Function Attrs: noinline nounwind uwtable
define internal double @calledSum.1(double %a, double %b) #0 !taffo.initweight !27 !taffo.sourceFunction !19 !taffo.funinfo !28 {
entry:
  %a.addr = alloca double, align 8
  %b.addr = alloca double, align 8
  store double %a, double* %a.addr, align 8
  store double %b, double* %b.addr, align 8
  %0 = load double, double* %a.addr, align 8
  %1 = load double, double* %b.addr, align 8
  %add = fadd double %0, %1
  ret double %add
}

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.1 (tags/RELEASE_801/final)"}
!2 = !{i1 true}
!3 = !{}
!4 = !{i32 0}
!5 = !{!"a"}
!6 = !{i1 false, !7, !8, i2 1}
!7 = !{double 1.000000e+00, double 2.500000e+01}
!8 = !{double 8.000000e-01}
!9 = !{!"b"}
!10 = !{i1 false, !11, !12, i2 1}
!11 = !{double 1.000000e+00, double 2.000000e+00}
!12 = !{double 9.000000e-01}
!13 = !{i32 1}
!14 = !{i32 2}
!15 = !{i32 3}
!16 = !{i32 4}
!17 = !{i32 5}
!18 = !{i32 6}
!19 = !{double (double, double)* @calledSum}
!20 = !{i32 -1, i32 -1, i32 -1, i32 -1}
!21 = !{i32 0, i1 false, i32 0, i1 false, i32 0, i1 false, i32 0, i1 false}
!22 = !{i32 -1}
!23 = !{i32 0, i1 false}
!24 = !{i32 -1, i32 -1}
!25 = !{double (double, double)* @calledSum.1}
!26 = !{i32 0, i1 false, i32 0, i1 false}
!27 = !{i32 3, i32 -1}
!28 = !{i32 1, !10, i32 0, i1 false}
