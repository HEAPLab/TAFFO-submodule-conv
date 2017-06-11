; ModuleID = 'test3.c'
source_filename = "test3.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

@random.seed = internal unnamed_addr global i32 123456, align 4
@.str = private unnamed_addr constant [9 x i8] c"no_float\00", section "llvm.metadata"
@.str.1 = private unnamed_addr constant [8 x i8] c"test3.c\00", section "llvm.metadata"

; Function Attrs: norecurse nounwind ssp uwtable
define float @random() local_unnamed_addr #0 {
  %1 = load i32, i32* @random.seed, align 4, !tbaa !2
  %2 = mul i32 %1, -928963441
  %3 = add i32 %2, 42
  %4 = urem i32 %3, -2
  store i32 %4, i32* @random.seed, align 4, !tbaa !2
  %5 = uitofp i32 %4 to double
  %6 = fdiv double %5, 0x41EFFFFFFFE00000
  %7 = fptrunc double %6 to float
  ret float %7
}

; Function Attrs: nounwind ssp uwtable
define float @test(i32, i32, float, float, float) local_unnamed_addr #1 {
  %6 = alloca float, align 4
  %7 = bitcast float* %6 to i8*
  call void @llvm.lifetime.start(i64 4, i8* %7) #3
  call void @llvm.var.annotation(i8* %7, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str, i64 0, i64 0), i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.1, i64 0, i64 0), i32 13)
  %8 = icmp eq i32 %0, 0
  %9 = select i1 %8, float 1.500000e+00, float %4
  store float %9, float* %6, align 4, !tbaa !6
  %10 = call float @random()
  %11 = load float, float* %6, align 4, !tbaa !6
  %12 = fmul float %10, %11
  store float %12, float* %6, align 4, !tbaa !6
  %13 = icmp eq i32 %1, 0
  br i1 %13, label %19, label %14

; <label>:14:                                     ; preds = %5
  %15 = fadd float %12, %2
  store float %15, float* %6, align 4, !tbaa !6
  %16 = call float @random()
  %17 = load float, float* %6, align 4, !tbaa !6
  %18 = fmul float %16, %17
  br label %23

; <label>:19:                                     ; preds = %5
  %20 = call float @random()
  %21 = load float, float* %6, align 4, !tbaa !6
  %22 = fdiv float %21, %20
  br label %23

; <label>:23:                                     ; preds = %19, %14
  %24 = phi float [ %22, %19 ], [ %18, %14 ]
  call void @llvm.lifetime.end(i64 4, i8* nonnull %7) #3
  ret float %24
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #2

; Function Attrs: nounwind
declare void @llvm.var.annotation(i8*, i8*, i8*, i32) #3

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #2

attributes #0 = { norecurse nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { argmemonly nounwind }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"PIC Level", i32 2}
!1 = !{!"Apple LLVM version 8.1.0 (clang-802.0.42)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"float", !4, i64 0}
