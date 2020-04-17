; ModuleID = 'a.1.magiclangtmp.ll'
source_filename = "simple.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [20 x i8] c"scalar(range(1,25))\00", section "llvm.metadata"
@.str.1 = private unnamed_addr constant [9 x i8] c"simple.c\00", section "llvm.metadata"
@.str.2 = private unnamed_addr constant [19 x i8] c"scalar(range(1,2))\00", section "llvm.metadata"
@.str.3 = private unnamed_addr constant [1 x i8] zeroinitializer, section "llvm.metadata"
@.str.4 = private unnamed_addr constant [3 x i8] c"%f\00", align 1
@.str.5 = private unnamed_addr constant [7 x i8] c"%f, %d\00", align 1
@.str.6 = private unnamed_addr constant [7 x i8] c"%d, %f\00", align 1
@.str.7 = private unnamed_addr constant [3 x i8] c"%d\00", align 1

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @main() #0 !taffo.initweight !2 !taffo.funinfo !2 {
entry:
  %retval = alloca i32, align 4
  %a = alloca double, align 8, !taffo.initweight !3, !taffo.info !4
  %b = alloca double, align 8, !taffo.initweight !3, !taffo.info !6
  %c = alloca double, align 8
  %d = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %a1 = bitcast double* %a to i8*, !taffo.initweight !8, !taffo.info !4
  %b2 = bitcast double* %b to i8*, !taffo.initweight !8, !taffo.info !6
  %c3 = bitcast double* %c to i8*
  call void @llvm.var.annotation(i8* %c3, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str.3, i32 0, i32 0), i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str.1, i32 0, i32 0), i32 7)
  %call = call i32 (i8*, ...) @__isoc99_scanf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.4, i32 0, i32 0), double* %a), !taffo.initweight !8, !taffo.info !4
  %call4 = call i32 (i8*, ...) @__isoc99_scanf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.4, i32 0, i32 0), double* %b), !taffo.initweight !8, !taffo.info !6
  %0 = load double, double* %a, align 8, !taffo.initweight !8, !taffo.info !4
  %1 = load double, double* %b, align 8, !taffo.initweight !8, !taffo.info !6
  %mul = fmul double %0, %1, !taffo.initweight !9, !taffo.info !4
  %2 = load double, double* %a, align 8, !taffo.initweight !8, !taffo.info !4
  %div = fdiv double %mul, %2, !taffo.initweight !9, !taffo.info !4
  %call5 = call double @exp(double %div) #1, !taffo.initweight !10, !taffo.info !4
  store double %call5, double* %c, align 8, !taffo.initweight !11, !taffo.info !4
  store i32 0, i32* %d, align 4
  %3 = load double, double* %a, align 8, !taffo.initweight !8, !taffo.info !4
  %4 = load double, double* %b, align 8, !taffo.initweight !8, !taffo.info !6
  %cmp = fcmp oeq double %3, %4, !taffo.initweight !9, !taffo.info !4
  br i1 %cmp, label %if.then, label %if.else, !taffo.initweight !10, !taffo.info !4

if.then:                                          ; preds = %entry
  %5 = load double, double* %c, align 8
  %conv = fptosi double %5 to i32
  store i32 %conv, i32* %d, align 4
  %6 = load double, double* %c, align 8
  %call6 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str.5, i32 0, i32 0), double %6, i32 2)
  br label %if.end

if.else:                                          ; preds = %entry
  %7 = load double, double* %a, align 8, !taffo.initweight !8, !taffo.info !4
  %conv7 = fptosi double %7 to i32, !taffo.initweight !9, !taffo.info !4
  store i32 %conv7, i32* %d, align 4, !taffo.initweight !10, !taffo.info !4
  %8 = load double, double* %c, align 8
  %call8 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str.6, i32 0, i32 0), i32 3, double %8)
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %9 = load i32, i32* %d, align 4
  %call9 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.7, i32 0, i32 0), i32 %9)
  %10 = load double, double* %b, align 8, !taffo.initweight !8, !taffo.info !6
  %11 = load i32, i32* %d, align 4
  %conv10 = sitofp i32 %11 to double
  %call11 = call double @calledSum.1(double %10, double %conv10), !taffo.initweight !9, !taffo.info !6, !taffo.originalCall !12
  %call12 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.7, i32 0, i32 0), double %call11), !taffo.initweight !10, !taffo.info !6
  ret i32 0
}

; Function Attrs: nounwind
declare !taffo.initweight !13 !taffo.funinfo !14 void @llvm.var.annotation(i8*, i8*, i8*, i32) #1

declare !taffo.initweight !15 !taffo.funinfo !16 dso_local i32 @__isoc99_scanf(i8*, ...) #2

; Function Attrs: nounwind
declare !taffo.initweight !15 !taffo.funinfo !16 dso_local double @exp(double) #3

declare !taffo.initweight !15 !taffo.funinfo !16 dso_local i32 @printf(i8*, ...) #2

; Function Attrs: noinline nounwind uwtable
define dso_local double @calledSum(double %a, double %b) #0 !taffo.initweight !17 !taffo.equivalentChild !18 !taffo.funinfo !19 {
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
define internal double @calledSum.1(double %a, double %b) #0 !taffo.initweight !20 !taffo.sourceFunction !12 !taffo.funinfo !21 {
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
!2 = !{}
!3 = !{i32 0}
!4 = !{i1 false, !5, i1 false, i2 1}
!5 = !{double 1.000000e+00, double 2.500000e+01}
!6 = !{i1 false, !7, i1 false, i2 1}
!7 = !{double 1.000000e+00, double 2.000000e+00}
!8 = !{i32 1}
!9 = !{i32 2}
!10 = !{i32 3}
!11 = !{i32 4}
!12 = !{double (double, double)* @calledSum}
!13 = !{i32 -1, i32 -1, i32 -1, i32 -1}
!14 = !{i32 0, i1 false, i32 0, i1 false, i32 0, i1 false, i32 0, i1 false}
!15 = !{i32 -1}
!16 = !{i32 0, i1 false}
!17 = !{i32 -1, i32 -1}
!18 = !{double (double, double)* @calledSum.1}
!19 = !{i32 0, i1 false, i32 0, i1 false}
!20 = !{i32 2, i32 -1}
!21 = !{i32 1, !6, i32 0, i1 false}
