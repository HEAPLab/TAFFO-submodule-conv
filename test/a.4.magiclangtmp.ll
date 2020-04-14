; ModuleID = 'a.3.magiclangtmp.ll'
source_filename = "simple.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str.1 = private unnamed_addr constant [9 x i8] c"simple.c\00", section "llvm.metadata", !taffo.info !0
@.str.3 = private unnamed_addr constant [12 x i8] c"target('c')\00", section "llvm.metadata", !taffo.info !2
@.str.4 = private unnamed_addr constant [3 x i8] c"%f\00", align 1, !taffo.info !4
@.str.5 = private unnamed_addr constant [7 x i8] c"%f, %d\00", align 1, !taffo.info !4
@.str.6 = private unnamed_addr constant [7 x i8] c"%d, %f\00", align 1, !taffo.info !4
@.str.7 = private unnamed_addr constant [3 x i8] c"%d\00", align 1, !taffo.info !6

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @main() #0 !taffo.start !10 !taffo.initweight !11 !taffo.funinfo !11 {
entry:
  %a = alloca float, align 4, !taffo.info !12, !taffo.initweight !16, !taffo.target !17
  %b = alloca float, align 4, !taffo.info !18, !taffo.initweight !16, !taffo.target !22
  %c = alloca float, align 4, !taffo.info !23
  %a1 = bitcast float* %a to i8*, !taffo.info !25, !taffo.initweight !26, !taffo.target !17
  %b2 = bitcast float* %b to i8*, !taffo.info !27, !taffo.initweight !26, !taffo.target !22
  %c3 = bitcast float* %c to i8*
  call void @llvm.var.annotation(i8* %c3, i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str.3, i32 0, i32 0), i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str.1, i32 0, i32 0), i32 6), !taffo.constinfo !28
  %call = call i32 (i8*, ...) @__isoc99_scanf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.4, i32 0, i32 0), float* %a), !taffo.info !25, !taffo.initweight !26, !taffo.target !17, !taffo.constinfo !29
  %call4 = call i32 (i8*, ...) @__isoc99_scanf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.4, i32 0, i32 0), float* %b), !taffo.info !27, !taffo.initweight !26, !taffo.target !22, !taffo.constinfo !29
  %0 = load float, float* %a, align 4, !taffo.info !12, !taffo.initweight !26, !taffo.target !17
  %1 = load float, float* %b, align 4, !taffo.info !18, !taffo.initweight !26, !taffo.target !22
  %mul = fmul float %0, %1, !taffo.info !30, !taffo.initweight !33, !taffo.target !17
  %2 = load float, float* %a, align 4, !taffo.info !12, !taffo.initweight !26, !taffo.target !17
  %div = fdiv float %mul, %2, !taffo.info !34, !taffo.initweight !33, !taffo.target !17
  %conv = fpext float %div to double, !taffo.info !34, !taffo.initweight !36, !taffo.target !17
  %call5 = call double @exp(double %conv) #1, !taffo.info !37, !taffo.initweight !40, !taffo.target !17, !taffo.constinfo !41
  %conv6 = fptrunc double %call5 to float, !taffo.info !42, !taffo.initweight !44, !taffo.target !17
  store float %conv6, float* %c, align 4, !taffo.info !45, !taffo.initweight !46, !taffo.target !17
  %3 = load float, float* %a, align 4, !taffo.info !12, !taffo.initweight !26, !taffo.target !17
  %4 = load float, float* %b, align 4, !taffo.info !18, !taffo.initweight !26, !taffo.target !22
  %cmp = fcmp oeq float %3, %4, !taffo.info !25, !taffo.initweight !33, !taffo.target !17
  br i1 %cmp, label %if.then, label %if.else, !taffo.info !45, !taffo.initweight !36, !taffo.target !17

if.then:                                          ; preds = %entry
  %5 = load float, float* %c, align 4, !taffo.info !23
  %conv8 = fptosi float %5 to i32, !taffo.info !47
  %6 = load float, float* %c, align 4, !taffo.info !23
  %conv9 = fpext float %6 to double, !taffo.info !23
  %call10 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str.5, i32 0, i32 0), double %conv9, i32 2), !taffo.constinfo !49
  br label %if.end

if.else:                                          ; preds = %entry
  %7 = load float, float* %a, align 4, !taffo.info !12, !taffo.initweight !26, !taffo.target !17
  %conv11 = fptosi float %7 to i32, !taffo.info !25, !taffo.initweight !33, !taffo.target !17
  %8 = load float, float* %c, align 4, !taffo.info !23
  %conv12 = fpext float %8 to double, !taffo.info !23
  %call13 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str.6, i32 0, i32 0), i32 3, double %conv12), !taffo.constinfo !49
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %d.0 = phi i32 [ %conv8, %if.then ], [ %conv11, %if.else ], !taffo.info !50
  %call14 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.7, i32 0, i32 0), i32 %d.0), !taffo.constinfo !29
  %9 = load float, float* %b, align 4, !taffo.info !18, !taffo.initweight !26, !taffo.target !22
  %conv15 = fpext float %9 to double, !taffo.info !18, !taffo.initweight !33, !taffo.target !22
  %conv16 = sitofp i32 %d.0 to double, !taffo.info !50
  %call17 = call double @calledSum.1(double %conv15, double %conv16), !taffo.info !51, !taffo.initweight !36, !taffo.target !22, !taffo.constinfo !29, !taffo.originalCall !54
  %call18 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.7, i32 0, i32 0), double %call17), !taffo.info !55, !taffo.initweight !40, !taffo.target !22, !taffo.constinfo !29
  ret i32 0, !taffo.info !56
}

; Function Attrs: nounwind
declare !taffo.initweight !58 !taffo.funinfo !59 void @llvm.var.annotation(i8*, i8*, i8*, i32) #1

declare !taffo.initweight !60 !taffo.funinfo !61 dso_local i32 @__isoc99_scanf(i8*, ...) #2

; Function Attrs: nounwind
declare !taffo.initweight !60 !taffo.funinfo !61 dso_local double @exp(double) #3

declare !taffo.initweight !60 !taffo.funinfo !61 dso_local i32 @printf(i8*, ...) #2

; Function Attrs: noinline nounwind uwtable
define dso_local double @calledSum(double %a, double %b) #0 !taffo.initweight !62 !taffo.funinfo !63 !taffo.equivalentChild !64 {
entry:
  %add = fadd double %a, %b
  ret double %add
}

; Function Attrs: noinline nounwind uwtable
define internal double @calledSum.1(double %a, double %b) #0 !taffo.initweight !65 !taffo.funinfo !66 !taffo.sourceFunction !54 {
entry:
  %add = fadd double %a, %b, !taffo.info !67
  ret double %add, !taffo.info !67
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
!3 = !{double 0.000000e+00, double 1.160000e+02}
!4 = !{i1 false, !5, i1 false, i2 0}
!5 = !{double 0.000000e+00, double 1.020000e+02}
!6 = !{i1 false, !7, i1 false, i2 0}
!7 = !{double 0.000000e+00, double 1.000000e+02}
!8 = !{i32 1, !"wchar_size", i32 4}
!9 = !{!"clang version 8.0.1 (tags/RELEASE_801/final)"}
!10 = !{i1 true}
!11 = !{}
!12 = !{!13, !14, !15, i2 1}
!13 = !{!"float", i32 0, double 2.500000e+01}
!14 = !{double 1.000000e+00, double 2.500000e+01}
!15 = !{double 8.000000e-01}
!16 = !{i32 0}
!17 = !{!"a"}
!18 = !{!19, !20, !21, i2 1}
!19 = !{!"float", i32 0, double 2.000000e+00}
!20 = !{double 1.000000e+00, double 2.000000e+00}
!21 = !{double 9.000000e-01}
!22 = !{!"b"}
!23 = !{i1 false, !24, i1 false, i2 0}
!24 = !{double 0x3FF0A72920000000, double 0x4471910400000000}
!25 = !{!13, i1 false, !15, i2 1}
!26 = !{i32 1}
!27 = !{!19, i1 false, !21, i2 1}
!28 = !{i1 false, i1 false, i1 false, i1 false, i1 false}
!29 = !{i1 false, i1 false, i1 false}
!30 = !{!31, !32, !15, i2 1}
!31 = !{!"float", i32 0, double 5.000000e+01}
!32 = !{double 1.000000e+00, double 5.000000e+01}
!33 = !{i32 2}
!34 = !{!31, !35, !15, i2 1}
!35 = !{double 4.000000e-02, double 5.000000e+01}
!36 = !{i32 3}
!37 = !{!38, !39, !15, i2 1}
!38 = !{!"float", i32 0, double 0x44719103E4080B45}
!39 = !{double 0x3FF0A72932C7B125, double 0x44719103E4080B45}
!40 = !{i32 4}
!41 = !{i1 false, i1 false}
!42 = !{!43, !24, !15, i2 1}
!43 = !{!"float", i32 0, double 0x4471910400000000}
!44 = !{i32 5}
!45 = !{i1 false, !14, !15, i2 1}
!46 = !{i32 6}
!47 = !{i1 false, !48, i1 false, i2 0}
!48 = !{double 1.000000e+00, double 0xC3E0000000000000}
!49 = !{i1 false, i1 false, i1 false, i1 false}
!50 = !{i1 false, !14, i1 false, i2 0}
!51 = !{!52, !53, !21, i2 1}
!52 = !{!"float", i32 0, double 2.700000e+01}
!53 = !{double 2.000000e+00, double 2.700000e+01}
!54 = !{double (double, double)* @calledSum}
!55 = !{!52, i1 false, !21, i2 1}
!56 = !{i1 false, !57, i1 false, i2 0}
!57 = !{double 0.000000e+00, double 0.000000e+00}
!58 = !{i32 -1, i32 -1, i32 -1, i32 -1}
!59 = !{i32 0, i1 false, i32 0, i1 false, i32 0, i1 false, i32 0, i1 false}
!60 = !{i32 -1}
!61 = !{i32 0, i1 false}
!62 = !{i32 -1, i32 -1}
!63 = !{i32 0, i1 false, i32 0, i1 false}
!64 = !{double (double, double)* @calledSum.1}
!65 = !{i32 3, i32 -1}
!66 = !{i32 1, !18, i32 1, !50}
!67 = !{i1 false, !53, i1 false, i2 0}
