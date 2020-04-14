; ModuleID = 'a.2.magiclangtmp.ll'
source_filename = "simple.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [43 x i8] c"target('a') scalar(range(1,25) error(0.8))\00", section "llvm.metadata", !taffo.info !0
@.str.1 = private unnamed_addr constant [9 x i8] c"simple.c\00", section "llvm.metadata", !taffo.info !2
@.str.2 = private unnamed_addr constant [42 x i8] c"target('b') scalar(range(1,2) error(0.9))\00", section "llvm.metadata", !taffo.info !0
@.str.3 = private unnamed_addr constant [12 x i8] c"target('c')\00", section "llvm.metadata", !taffo.info !0
@.str.4 = private unnamed_addr constant [3 x i8] c"%f\00", align 1, !taffo.info !4
@.str.5 = private unnamed_addr constant [7 x i8] c"%f, %d\00", align 1, !taffo.info !4
@.str.6 = private unnamed_addr constant [7 x i8] c"%d, %f\00", align 1, !taffo.info !4
@.str.7 = private unnamed_addr constant [3 x i8] c"%d\00", align 1, !taffo.info !6

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @main() #0 !taffo.start !10 !taffo.initweight !11 !taffo.funinfo !11 {
entry:
  %a = alloca float, align 4, !taffo.initweight !12, !taffo.target !13, !taffo.info !14
  %b = alloca float, align 4, !taffo.initweight !12, !taffo.target !17, !taffo.info !18
  %c = alloca float, align 4, !taffo.info !21
  %a1 = bitcast float* %a to i8*, !taffo.initweight !23, !taffo.target !13, !taffo.info !14
  %b2 = bitcast float* %b to i8*, !taffo.initweight !23, !taffo.target !17, !taffo.info !18
  %c3 = bitcast float* %c to i8*
  call void @llvm.var.annotation(i8* %c3, i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str.3, i32 0, i32 0), i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str.1, i32 0, i32 0), i32 6), !taffo.constinfo !24
  %call = call i32 (i8*, ...) @__isoc99_scanf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.4, i32 0, i32 0), float* %a), !taffo.initweight !23, !taffo.target !13, !taffo.info !14, !taffo.constinfo !25
  %call4 = call i32 (i8*, ...) @__isoc99_scanf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.4, i32 0, i32 0), float* %b), !taffo.initweight !23, !taffo.target !17, !taffo.info !18, !taffo.constinfo !25
  %0 = load float, float* %a, align 4, !taffo.initweight !23, !taffo.target !13, !taffo.info !14
  %1 = load float, float* %b, align 4, !taffo.initweight !23, !taffo.target !17, !taffo.info !18
  %mul = fmul float %0, %1, !taffo.initweight !26, !taffo.target !13, !taffo.info !27
  %2 = load float, float* %a, align 4, !taffo.initweight !23, !taffo.target !13, !taffo.info !14
  %div = fdiv float %mul, %2, !taffo.initweight !26, !taffo.target !13, !taffo.info !29
  %conv = fpext float %div to double, !taffo.initweight !31, !taffo.target !13, !taffo.info !29
  %call5 = call double @exp(double %conv) #1, !taffo.initweight !32, !taffo.target !13, !taffo.info !33, !taffo.constinfo !35
  %conv6 = fptrunc double %call5 to float, !taffo.initweight !36, !taffo.target !13, !taffo.info !37
  store float %conv6, float* %c, align 4, !taffo.initweight !38, !taffo.target !13, !taffo.info !14
  %3 = load float, float* %a, align 4, !taffo.initweight !23, !taffo.target !13, !taffo.info !14
  %4 = load float, float* %b, align 4, !taffo.initweight !23, !taffo.target !17, !taffo.info !18
  %cmp = fcmp oeq float %3, %4, !taffo.initweight !26, !taffo.target !13, !taffo.info !39
  br i1 %cmp, label %if.then, label %if.else, !taffo.initweight !31, !taffo.target !13, !taffo.info !14

if.then:                                          ; preds = %entry
  %5 = load float, float* %c, align 4, !taffo.info !21
  %conv8 = fptosi float %5 to i32, !taffo.info !41
  %6 = load float, float* %c, align 4, !taffo.info !21
  %conv9 = fpext float %6 to double, !taffo.info !21
  %call10 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str.5, i32 0, i32 0), double %conv9, i32 2), !taffo.constinfo !43
  br label %if.end

if.else:                                          ; preds = %entry
  %7 = load float, float* %a, align 4, !taffo.initweight !23, !taffo.target !13, !taffo.info !14
  %conv11 = fptosi float %7 to i32, !taffo.initweight !26, !taffo.target !13, !taffo.info !14
  %8 = load float, float* %c, align 4, !taffo.info !21
  %conv12 = fpext float %8 to double, !taffo.info !21
  %call13 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str.6, i32 0, i32 0), i32 3, double %conv12), !taffo.constinfo !43
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %d.0 = phi i32 [ %conv8, %if.then ], [ %conv11, %if.else ], !taffo.info !44
  %call14 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.7, i32 0, i32 0), i32 %d.0), !taffo.constinfo !25
  %9 = load float, float* %b, align 4, !taffo.initweight !23, !taffo.target !17, !taffo.info !18
  %conv15 = fpext float %9 to double, !taffo.initweight !26, !taffo.target !17, !taffo.info !18
  %conv16 = sitofp i32 %d.0 to double, !taffo.info !44
  %call17 = call double @calledSum.1(double %conv15, double %conv16), !taffo.initweight !31, !taffo.target !17, !taffo.info !45, !taffo.originalCall !47, !taffo.constinfo !25
  %call18 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.7, i32 0, i32 0), double %call17), !taffo.initweight !32, !taffo.target !17, !taffo.info !18, !taffo.constinfo !25
  ret i32 0, !taffo.info !48
}

; Function Attrs: nounwind
declare !taffo.initweight !50 !taffo.funinfo !51 void @llvm.var.annotation(i8*, i8*, i8*, i32) #1

declare !taffo.initweight !52 !taffo.funinfo !53 dso_local i32 @__isoc99_scanf(i8*, ...) #2

; Function Attrs: nounwind
declare !taffo.initweight !52 !taffo.funinfo !53 dso_local double @exp(double) #3

declare !taffo.initweight !52 !taffo.funinfo !53 dso_local i32 @printf(i8*, ...) #2

; Function Attrs: noinline nounwind uwtable
define dso_local double @calledSum(double %a, double %b) #0 !taffo.initweight !54 !taffo.funinfo !55 !taffo.equivalentChild !56 {
entry:
  %add = fadd double %a, %b
  ret double %add
}

; Function Attrs: noinline nounwind uwtable
define internal double @calledSum.1(double %a, double %b) #0 !taffo.initweight !57 !taffo.funinfo !58 !taffo.sourceFunction !47 {
entry:
  %add = fadd double %a, %b, !taffo.info !59
  ret double %add, !taffo.info !59
}

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!8}
!llvm.ident = !{!9}

!0 = !{i1 false, !1, i1 false, i2 0}
!1 = !{double 0.000000e+00, double 1.160000e+02}
!2 = !{i1 false, !3, i1 false, i2 0}
!3 = !{double 0.000000e+00, double 1.150000e+02}
!4 = !{i1 false, !5, i1 false, i2 0}
!5 = !{double 0.000000e+00, double 1.020000e+02}
!6 = !{i1 false, !7, i1 false, i2 0}
!7 = !{double 0.000000e+00, double 1.000000e+02}
!8 = !{i32 1, !"wchar_size", i32 4}
!9 = !{!"clang version 8.0.1 (tags/RELEASE_801/final)"}
!10 = !{i1 true}
!11 = !{}
!12 = !{i32 0}
!13 = !{!"a"}
!14 = !{i1 false, !15, !16, i2 1}
!15 = !{double 1.000000e+00, double 2.500000e+01}
!16 = !{double 8.000000e-01}
!17 = !{!"b"}
!18 = !{i1 false, !19, !20, i2 1}
!19 = !{double 1.000000e+00, double 2.000000e+00}
!20 = !{double 9.000000e-01}
!21 = !{i1 false, !22, i1 false, i2 0}
!22 = !{double 0x3FF0A72920000000, double 0x4471910400000000}
!23 = !{i32 1}
!24 = !{i1 false, i1 false, i1 false, i1 false, i1 false}
!25 = !{i1 false, i1 false, i1 false}
!26 = !{i32 2}
!27 = !{i1 false, !28, !16, i2 1}
!28 = !{double 1.000000e+00, double 5.000000e+01}
!29 = !{i1 false, !30, !16, i2 1}
!30 = !{double 4.000000e-02, double 5.000000e+01}
!31 = !{i32 3}
!32 = !{i32 4}
!33 = !{i1 false, !34, !16, i2 1}
!34 = !{double 0x3FF0A72932C7B125, double 0x44719103E4080B45}
!35 = !{i1 false, i1 false}
!36 = !{i32 5}
!37 = !{i1 false, !22, !16, i2 1}
!38 = !{i32 6}
!39 = !{i1 false, !40, !16, i2 1}
!40 = !{double 0.000000e+00, double 1.000000e+00}
!41 = !{i1 false, !42, i1 false, i2 0}
!42 = !{double 1.000000e+00, double 0xC3E0000000000000}
!43 = !{i1 false, i1 false, i1 false, i1 false}
!44 = !{i1 false, !15, i1 false, i2 0}
!45 = !{i1 false, !46, !20, i2 1}
!46 = !{double 2.000000e+00, double 2.700000e+01}
!47 = !{double (double, double)* @calledSum}
!48 = !{i1 false, !49, i1 false, i2 0}
!49 = !{double 0.000000e+00, double 0.000000e+00}
!50 = !{i32 -1, i32 -1, i32 -1, i32 -1}
!51 = !{i32 0, i1 false, i32 0, i1 false, i32 0, i1 false, i32 0, i1 false}
!52 = !{i32 -1}
!53 = !{i32 0, i1 false}
!54 = !{i32 -1, i32 -1}
!55 = !{i32 0, i1 false, i32 0, i1 false}
!56 = !{double (double, double)* @calledSum.1}
!57 = !{i32 3, i32 -1}
!58 = !{i32 1, !18, i32 1, !44}
!59 = !{i1 false, !46, i1 false, i2 0}
