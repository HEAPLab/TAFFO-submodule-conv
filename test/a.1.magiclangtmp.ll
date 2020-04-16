; ModuleID = 'simple.c'
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

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %a = alloca float, align 4
  %b = alloca float, align 4
  %c = alloca float, align 4
  %d = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %a1 = bitcast float* %a to i8*
  call void @llvm.var.annotation(i8* %a1, i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str.1, i32 0, i32 0), i32 5)
  %b2 = bitcast float* %b to i8*
  call void @llvm.var.annotation(i8* %b2, i8* getelementptr inbounds ([19 x i8], [19 x i8]* @.str.2, i32 0, i32 0), i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str.1, i32 0, i32 0), i32 6)
  %c3 = bitcast float* %c to i8*
  call void @llvm.var.annotation(i8* %c3, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str.3, i32 0, i32 0), i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str.1, i32 0, i32 0), i32 7)
  %call = call i32 (i8*, ...) @__isoc99_scanf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.4, i32 0, i32 0), float* %a)
  %call4 = call i32 (i8*, ...) @__isoc99_scanf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.4, i32 0, i32 0), float* %b)
  %0 = load float, float* %a, align 4
  %1 = load float, float* %b, align 4
  %mul = fmul float %0, %1
  %2 = load float, float* %a, align 4
  %div = fdiv float %mul, %2
  %conv = fpext float %div to double
  %call5 = call double @exp(double %conv) #1
  %conv6 = fptrunc double %call5 to float
  store float %conv6, float* %c, align 4
  store i32 0, i32* %d, align 4
  %3 = load float, float* %a, align 4
  %4 = load float, float* %b, align 4
  %cmp = fcmp oeq float %3, %4
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %5 = load float, float* %c, align 4
  %conv8 = fptosi float %5 to i32
  store i32 %conv8, i32* %d, align 4
  %6 = load float, float* %c, align 4
  %conv9 = fpext float %6 to double
  %call10 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str.5, i32 0, i32 0), double %conv9, i32 2)
  br label %if.end

if.else:                                          ; preds = %entry
  %7 = load float, float* %a, align 4
  %conv11 = fptosi float %7 to i32
  store i32 %conv11, i32* %d, align 4
  %8 = load float, float* %c, align 4
  %conv12 = fpext float %8 to double
  %call13 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str.6, i32 0, i32 0), i32 3, double %conv12)
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %9 = load i32, i32* %d, align 4
  %call14 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.7, i32 0, i32 0), i32 %9)
  %10 = load float, float* %b, align 4
  %conv15 = fpext float %10 to double
  %11 = load i32, i32* %d, align 4
  %conv16 = sitofp i32 %11 to double
  %call17 = call double @calledSum(double %conv15, double %conv16)
  %call18 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.7, i32 0, i32 0), double %call17)
  ret i32 0
}

; Function Attrs: nounwind
declare void @llvm.var.annotation(i8*, i8*, i8*, i32) #1

declare dso_local i32 @__isoc99_scanf(i8*, ...) #2

; Function Attrs: nounwind
declare dso_local double @exp(double) #3

declare dso_local i32 @printf(i8*, ...) #2

; Function Attrs: noinline nounwind optnone uwtable
define dso_local double @calledSum(double %a, double %b) #0 {
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

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.1 (tags/RELEASE_801/final)"}
