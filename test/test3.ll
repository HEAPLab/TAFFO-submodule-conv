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
define float @test(i32, i32, i32, float, float, float) local_unnamed_addr #1 {
  %7 = alloca float, align 4
  %8 = bitcast float* %7 to i8*
  call void @llvm.lifetime.start(i64 4, i8* %8) #3
  call void @llvm.var.annotation(i8* %8, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str, i64 0, i64 0), i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.1, i64 0, i64 0), i32 16)
  %9 = icmp eq i32 %0, 0
  %10 = select i1 %9, float 1.500000e+00, float %5
  store float %10, float* %7, align 4, !tbaa !6
  %11 = call float @random()
  %12 = load float, float* %7, align 4, !tbaa !6
  %13 = fmul float %11, %12
  store float %13, float* %7, align 4, !tbaa !6
  %14 = icmp eq i32 %1, 0
  br i1 %14, label %20, label %15

; <label>:15:                                     ; preds = %6
  %16 = fadd float %13, %3
  store float %16, float* %7, align 4, !tbaa !6
  %17 = call float @random()
  %18 = load float, float* %7, align 4, !tbaa !6
  %19 = fmul float %17, %18
  br label %24

; <label>:20:                                     ; preds = %6
  %21 = call float @random()
  %22 = load float, float* %7, align 4, !tbaa !6
  %23 = fdiv float %22, %21
  br label %24

; <label>:24:                                     ; preds = %20, %15
  %25 = phi float [ %23, %20 ], [ %19, %15 ]
  call void @llvm.lifetime.end(i64 4, i8* nonnull %8) #3
  ret float %25
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #2

; Function Attrs: nounwind
declare void @llvm.var.annotation(i8*, i8*, i8*, i32) #3

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #2

; Function Attrs: nounwind ssp uwtable
define float @test2(i32) local_unnamed_addr #1 {
  %2 = alloca float, align 4
  %3 = bitcast float* %2 to i8*
  call void @llvm.lifetime.start(i64 4, i8* %3) #3
  call void @llvm.var.annotation(i8* %3, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str, i64 0, i64 0), i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.1, i64 0, i64 0), i32 34)
  store float 0.000000e+00, float* %2, align 4, !tbaa !6
  %4 = icmp eq i32 %0, 0
  br i1 %4, label %9, label %5

; <label>:5:                                      ; preds = %1
  %6 = call float @random()
  %7 = load float, float* %2, align 4, !tbaa !6
  %8 = fadd float %6, %7
  br label %9

; <label>:9:                                      ; preds = %1, %5
  %10 = phi float [ %8, %5 ], [ 2.000000e+00, %1 ]
  store float %10, float* %2, align 4, !tbaa !6
  %11 = call float @random()
  %12 = load float, float* %2, align 4, !tbaa !6
  %13 = fadd float %11, %12
  call void @llvm.lifetime.end(i64 4, i8* %3) #3
  ret float %13
}

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
