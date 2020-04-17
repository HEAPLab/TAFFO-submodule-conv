; ModuleID = 'a.4.magiclangtmp.ll'
source_filename = "array.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@sum = common dso_local global double 0.000000e+00, align 8, !taffo.info !0
@array = common dso_local global [10 x double] zeroinitializer, align 16, !taffo.info !2, !taffo.initweight !5
@.str = private unnamed_addr constant [23 x i8] c"scalar(range(7, 2000))\00", section "llvm.metadata", !taffo.info !6
@.str.1 = private unnamed_addr constant [8 x i8] c"array.c\00", section "llvm.metadata", !taffo.info !8
@llvm.global.annotations = appending global [1 x { i8*, i8*, i8*, i32 }] [{ i8*, i8*, i8*, i32 } { i8* bitcast ([10 x double]* @array to i8*), i8* getelementptr inbounds ([23 x i8], [23 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.1, i32 0, i32 0), i32 3 }], section "llvm.metadata"
@array.fixp = common global [10 x half] zeroinitializer, align 16, !taffo.info !2

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @main() #0 !taffo.initweight !12 !taffo.funinfo !12 {
entry:
  %0 = load double, double* @sum, align 8, !taffo.info !0
  store double %0, double* getelementptr inbounds ([10 x double], [10 x double]* @array, i64 0, i64 0), align 16, !taffo.info !13, !taffo.constinfo !14
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc1, %for.inc ]
  %cmp = icmp slt i32 %i.0, 10
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %idxprom = sext i32 %i.0 to i64
  %arrayidx.0flp = getelementptr inbounds [10 x half], [10 x half]* @array.fixp, i64 0, i64 %idxprom, !taffo.info !2
  %"0flp" = load half, half* %arrayidx.0flp, align 8, !taffo.info !2
  %inc.0flp = fadd half %"0flp", 0xH3C00, !taffo.info !2, !taffo.constinfo !15
  store half %inc.0flp, half* %arrayidx.0flp, align 8, !taffo.info !13
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc1 = add nsw i32 %i.0, 1, !taffo.constinfo !14
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret i32 0
}

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!10}
!llvm.ident = !{!11}

!0 = !{i1 false, !1, i1 false, i2 0}
!1 = !{double 0.000000e+00, double 0.000000e+00}
!2 = !{!3, !4, i1 false, i2 1}
!3 = !{!"float", i32 0, double 2.000000e+03}
!4 = !{double 7.000000e+00, double 2.000000e+03}
!5 = !{i32 0}
!6 = !{i1 false, !7, i1 false, i2 0}
!7 = !{double 0.000000e+00, double 1.150000e+02}
!8 = !{i1 false, !9, i1 false, i2 0}
!9 = !{double 0.000000e+00, double 1.210000e+02}
!10 = !{i32 1, !"wchar_size", i32 4}
!11 = !{!"clang version 8.0.1 (tags/RELEASE_801/final)"}
!12 = !{}
!13 = !{i1 false, !4, i1 false, i2 1}
!14 = !{i1 false, i1 false}
!15 = !{i1 false, !16}
!16 = !{i1 false, !17, i1 false, i2 0}
!17 = !{double 1.000000e+00, double 1.000000e+00}
