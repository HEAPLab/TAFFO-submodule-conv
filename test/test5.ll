; ModuleID = 'test5.c'
source_filename = "test5.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

@.str = private unnamed_addr constant [9 x i8] c"no_float\00", section "llvm.metadata"
@.str.1 = private unnamed_addr constant [8 x i8] c"test5.c\00", section "llvm.metadata"

; Function Attrs: noinline nounwind ssp uwtable
define float @fpextfptrunc(float %a, double %b) #0 {
entry:
  %a.addr = alloca float, align 4
  %b.addr = alloca double, align 8
  %c = alloca double, align 8
  store float %a, float* %a.addr, align 4
  %a.addr1 = bitcast float* %a.addr to i8*
  call void @llvm.var.annotation(i8* %a.addr1, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.1, i32 0, i32 0), i32 3)
  store double %b, double* %b.addr, align 8
  %b.addr2 = bitcast double* %b.addr to i8*
  call void @llvm.var.annotation(i8* %b.addr2, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.1, i32 0, i32 0), i32 3)
  %c3 = bitcast double* %c to i8*
  call void @llvm.var.annotation(i8* %c3, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.1, i32 0, i32 0), i32 5)
  store double 1.230000e+02, double* %c, align 8
  %0 = load float, float* %a.addr, align 4
  %conv = fpext float %0 to double
  %1 = load double, double* %c, align 8
  %add = fadd double %1, %conv
  store double %add, double* %c, align 8
  %2 = load double, double* %c, align 8
  %conv4 = fptrunc double %2 to float
  %conv5 = fpext float %conv4 to double
  %3 = load double, double* %b.addr, align 8
  %add6 = fadd double %conv5, %3
  %conv7 = fptrunc double %add6 to float
  ret float %conv7
}

; Function Attrs: nounwind
declare void @llvm.var.annotation(i8*, i8*, i8*, i32) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"PIC Level", i32 2}
!1 = !{!"clang version 4.0.0 (tags/RELEASE_400/final)"}
