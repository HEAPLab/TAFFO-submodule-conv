; ModuleID = 'a.1.magiclangtmp.ll'
source_filename = "array.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@sum = common dso_local global double 0.000000e+00, align 8
@array = common dso_local global [10 x double] zeroinitializer, align 16, !taffo.initweight !0, !taffo.info !1
@.str = private unnamed_addr constant [23 x i8] c"scalar(range(7, 2000))\00", section "llvm.metadata"
@.str.1 = private unnamed_addr constant [8 x i8] c"array.c\00", section "llvm.metadata"
@llvm.global.annotations = appending global [1 x { i8*, i8*, i8*, i32 }] [{ i8*, i8*, i8*, i32 } { i8* bitcast ([10 x double]* @array to i8*), i8* getelementptr inbounds ([23 x i8], [23 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.1, i32 0, i32 0), i32 3 }], section "llvm.metadata"

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @main() #0 !taffo.initweight !5 !taffo.funinfo !5 {
entry:
  %retval = alloca i32, align 4
  %x = alloca double, align 8
  %i = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  store double 0.000000e+00, double* %x, align 8
  %0 = load double, double* @sum, align 8
  store double %0, double* getelementptr inbounds ([10 x double], [10 x double]* @array, i64 0, i64 0), align 16, !taffo.initweight !6, !taffo.info !1
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %1 = load i32, i32* %i, align 4
  %cmp = icmp slt i32 %1, 10
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %2 = load i32, i32* %i, align 4
  %idxprom = sext i32 %2 to i64
  %arrayidx = getelementptr inbounds [10 x double], [10 x double]* @array, i64 0, i64 %idxprom, !taffo.initweight !7, !taffo.info !1
  %3 = load double, double* %arrayidx, align 8, !taffo.initweight !6, !taffo.info !1
  %inc = fadd double %3, 1.000000e+00, !taffo.initweight !8, !taffo.info !1
  store double %inc, double* %arrayidx, align 8, !taffo.initweight !6, !taffo.info !1
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %4 = load i32, i32* %i, align 4
  %inc1 = add nsw i32 %4, 1
  store i32 %inc1, i32* %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %5 = load i32, i32* %retval, align 4
  ret i32 %5
}

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!3}
!llvm.ident = !{!4}

!0 = !{i32 0}
!1 = !{i1 false, !2, i1 false, i2 1}
!2 = !{double 7.000000e+00, double 2.000000e+03}
!3 = !{i32 1, !"wchar_size", i32 4}
!4 = !{!"clang version 8.0.1 (tags/RELEASE_801/final)"}
!5 = !{}
!6 = !{i32 2}
!7 = !{i32 1}
!8 = !{i32 3}
