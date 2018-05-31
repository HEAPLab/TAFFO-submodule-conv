; ModuleID = 'clang_push_attr.c'
source_filename = "clang_push_attr.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [15 x i8] c"no_float 16 16\00", section "llvm.metadata"
@.str.1 = private unnamed_addr constant [18 x i8] c"clang_push_attr.c\00", section "llvm.metadata"
@.str.2 = private unnamed_addr constant [14 x i8] c"no_float 8 24\00", section "llvm.metadata"
@.str.3 = private unnamed_addr constant [4 x i8] c"%f\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %out = alloca float, align 4
  %a = alloca float, align 4
  %b = alloca float, align 4
  %c = alloca float, align 4
  %d = alloca float, align 4
  store i32 0, i32* %retval, align 4
  %a1 = bitcast float* %a to i8*
  call void @llvm.var.annotation(i8* %a1, i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str.1, i32 0, i32 0), i32 5)
  store float 1.000000e+01, float* %a, align 4
  %b2 = bitcast float* %b to i8*
  call void @llvm.var.annotation(i8* %b2, i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str.1, i32 0, i32 0), i32 6)
  %c3 = bitcast float* %c to i8*
  call void @llvm.var.annotation(i8* %c3, i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str.2, i32 0, i32 0), i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str.1, i32 0, i32 0), i32 10)
  store float 0x4000D1B720000000, float* %c, align 4
  %d4 = bitcast float* %d to i8*
  call void @llvm.var.annotation(i8* %d4, i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str.2, i32 0, i32 0), i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str.1, i32 0, i32 0), i32 11)
  %0 = load float, float* %a, align 4
  %conv = fpext float %0 to double
  %mul = fmul double %conv, 2.102400e-01
  %conv5 = fptrunc double %mul to float
  store float %conv5, float* %b, align 4
  %1 = load float, float* %b, align 4
  %div = fdiv float %1, 2.000000e+00
  store float %div, float* %b, align 4
  %2 = load float, float* %c, align 4
  %div6 = fdiv float %2, 2.000000e+00
  store float %div6, float* %c, align 4
  %3 = load float, float* %b, align 4
  %4 = load float, float* %c, align 4
  %add = fadd float %3, %4
  store float %add, float* %d, align 4
  %5 = load float, float* %d, align 4
  store float %5, float* %out, align 4
  %6 = load float, float* %out, align 4
  %conv7 = fpext float %6 to double
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str.3, i32 0, i32 0), double %conv7)
  ret i32 0
}

; Function Attrs: nounwind
declare void @llvm.var.annotation(i8*, i8*, i8*, i32) #1

declare i32 @printf(i8*, ...) #2

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.1 (https://git.llvm.org/git/clang.git/ 0e746072ed897a85b4f533ab050b9f506941a097) (https://git.llvm.org/git/llvm.git/ f1b37feef3d5f09dadf6a46fdb11fa7e4218cf6c)"}
