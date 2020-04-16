; ModuleID = 'a.3.magiclangtmp.ll'
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
  %a = alloca float, align 4, !taffo.info !11, !taffo.initweight !14
  %b = alloca float, align 4, !taffo.info !15, !taffo.initweight !14
  %c = alloca float, align 4
  %a1 = bitcast float* %a to i8*, !taffo.info !18, !taffo.initweight !19
  %b2 = bitcast float* %b to i8*, !taffo.info !20, !taffo.initweight !19
  %c3 = bitcast float* %c to i8*
  call void @llvm.var.annotation(i8* %c3, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str.3, i32 0, i32 0), i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str.1, i32 0, i32 0), i32 7), !taffo.constinfo !21
  %call = call i32 (i8*, ...) @__isoc99_scanf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.4, i32 0, i32 0), float* %a), !taffo.info !18, !taffo.initweight !19, !taffo.constinfo !22
  %call4 = call i32 (i8*, ...) @__isoc99_scanf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.4, i32 0, i32 0), float* %b), !taffo.info !20, !taffo.initweight !19, !taffo.constinfo !22
  %0 = load float, float* %a, align 4, !taffo.info !11, !taffo.initweight !19
  %1 = load float, float* %b, align 4, !taffo.info !15, !taffo.initweight !19
  %mul = fmul float %0, %1, !taffo.info !11, !taffo.initweight !23
  %2 = load float, float* %a, align 4, !taffo.info !11, !taffo.initweight !19
  %div = fdiv float %mul, %2, !taffo.info !11, !taffo.initweight !23
  %conv = fpext float %div to double, !taffo.info !11, !taffo.initweight !24
  %call5 = call double @exp(double %conv) #1, !taffo.info !11, !taffo.initweight !25, !taffo.constinfo !26
  %conv6 = fptrunc double %call5 to float, !taffo.info !11, !taffo.initweight !27
  store float %conv6, float* %c, align 4, !taffo.info !28, !taffo.initweight !29
  %3 = load float, float* %a, align 4, !taffo.info !11, !taffo.initweight !19
  %4 = load float, float* %b, align 4, !taffo.info !15, !taffo.initweight !19
  %cmp = fcmp oeq float %3, %4, !taffo.info !18, !taffo.initweight !23
  br i1 %cmp, label %if.then, label %if.else, !taffo.info !28, !taffo.initweight !24

if.then:                                          ; preds = %entry
  %5 = load float, float* %c, align 4
  %conv8 = fptosi float %5 to i32
  %6 = load float, float* %c, align 4
  %conv9 = fpext float %6 to double
  %call10 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str.5, i32 0, i32 0), double %conv9, i32 2), !taffo.constinfo !30
  br label %if.end

if.else:                                          ; preds = %entry
  %7 = load float, float* %a, align 4, !taffo.info !11, !taffo.initweight !19
  %conv11 = fptosi float %7 to i32, !taffo.info !18, !taffo.initweight !23
  %8 = load float, float* %c, align 4
  %conv12 = fpext float %8 to double
  %call13 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str.6, i32 0, i32 0), i32 3, double %conv12), !taffo.constinfo !30
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %d.0 = phi i32 [ %conv8, %if.then ], [ %conv11, %if.else ]
  %call14 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.7, i32 0, i32 0), i32 %d.0), !taffo.constinfo !22
  %9 = load float, float* %b, align 4, !taffo.info !15, !taffo.initweight !19
  %conv15 = fpext float %9 to double, !taffo.info !15, !taffo.initweight !23
  %conv16 = sitofp i32 %d.0 to double
  %call17 = call double @calledSum.1(double %conv15, double %conv16), !taffo.info !15, !taffo.initweight !24, !taffo.constinfo !22, !taffo.originalCall !31
  %call18 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.7, i32 0, i32 0), double %call17), !taffo.info !20, !taffo.initweight !25, !taffo.constinfo !22
  ret i32 0
}

; Function Attrs: nounwind
declare !taffo.initweight !32 !taffo.funinfo !33 void @llvm.var.annotation(i8*, i8*, i8*, i32) #1

declare !taffo.initweight !34 !taffo.funinfo !35 dso_local i32 @__isoc99_scanf(i8*, ...) #2

; Function Attrs: nounwind
declare !taffo.initweight !34 !taffo.funinfo !35 dso_local double @exp(double) #3

declare !taffo.initweight !34 !taffo.funinfo !35 dso_local i32 @printf(i8*, ...) #2

; Function Attrs: noinline nounwind uwtable
define dso_local double @calledSum(double %a, double %b) #0 !taffo.initweight !36 !taffo.funinfo !37 !taffo.equivalentChild !38 {
entry:
  %add = fadd double %a, %b
  ret double %add
}

; Function Attrs: noinline nounwind uwtable
define internal double @calledSum.1(double %a, double %b) #0 !taffo.initweight !39 !taffo.funinfo !40 !taffo.sourceFunction !31 {
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
!11 = !{!12, !13, i1 false, i2 1}
!12 = !{!"float", i32 0, double 2.500000e+01}
!13 = !{double 1.000000e+00, double 2.500000e+01}
!14 = !{i32 0}
!15 = !{!16, !17, i1 false, i2 1}
!16 = !{!"float", i32 0, double 2.000000e+00}
!17 = !{double 1.000000e+00, double 2.000000e+00}
!18 = !{!12, i1 false, i1 false, i2 1}
!19 = !{i32 1}
!20 = !{!16, i1 false, i1 false, i2 1}
!21 = !{i1 false, i1 false, i1 false, i1 false, i1 false}
!22 = !{i1 false, i1 false, i1 false}
!23 = !{i32 2}
!24 = !{i32 3}
!25 = !{i32 4}
!26 = !{i1 false, i1 false}
!27 = !{i32 5}
!28 = !{i1 false, !13, i1 false, i2 1}
!29 = !{i32 6}
!30 = !{i1 false, i1 false, i1 false, i1 false}
!31 = !{double (double, double)* @calledSum}
!32 = !{i32 -1, i32 -1, i32 -1, i32 -1}
!33 = !{i32 0, i1 false, i32 0, i1 false, i32 0, i1 false, i32 0, i1 false}
!34 = !{i32 -1}
!35 = !{i32 0, i1 false}
!36 = !{i32 -1, i32 -1}
!37 = !{i32 0, i1 false, i32 0, i1 false}
!38 = !{double (double, double)* @calledSum.1}
!39 = !{i32 3, i32 -1}
!40 = !{i32 1, !15, i32 0, i1 false}
