; ModuleID = 'cpp11attributes.cpp'
source_filename = "cpp11attributes.cpp"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.13.0"

; Function Attrs: noinline nounwind ssp uwtable
define float @_Z4oveniif(i32 %stuff, i32 %baked, float %cherry) #0 {
entry:
  %stuff.addr = alloca i32, align 4
  %baked.addr = alloca i32, align 4
  %cherry.addr = alloca float, align 4
  %cake = alloca float, align 4
  store i32 %stuff, i32* %stuff.addr, align 4
  store i32 %baked, i32* %baked.addr, align 4
  store float %cherry, float* %cherry.addr, align 4
  %0 = load i32, i32* %baked.addr, align 4
  %1 = load i32, i32* %stuff.addr, align 4
  %add = add nsw i32 %0, %1
  %conv = sitofp i32 %add to float
  store float %conv, float* %cake, align 4
  %2 = load float, float* %cake, align 4
  %3 = load float, float* %cherry.addr, align 4
  %add1 = fadd float %2, %3
  ret float %add1
}

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"PIC Level", i32 2}
!1 = !{!"clang version 4.0.0 (tags/RELEASE_400/final)"}
