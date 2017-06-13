; ModuleID = 'test4.c'
source_filename = "test4.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

@random.seed = internal unnamed_addr global i32 123456, align 4
@.str = private unnamed_addr constant [9 x i8] c"no_float\00", section "llvm.metadata"
@.str.1 = private unnamed_addr constant [8 x i8] c"test4.c\00", section "llvm.metadata"

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
define float @test(i32, i32, i32) local_unnamed_addr #1 {
  %4 = alloca float, align 4
  %5 = alloca float, align 4
  %6 = alloca float, align 4
  %7 = bitcast float* %4 to i8*
  call void @llvm.lifetime.start(i64 4, i8* %7) #3
  call void @llvm.var.annotation(i8* %7, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str, i64 0, i64 0), i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.1, i64 0, i64 0), i32 16)
  %8 = call float @random()
  store float %8, float* %4, align 4, !tbaa !6
  %9 = bitcast float* %5 to i8*
  call void @llvm.lifetime.start(i64 4, i8* %9) #3
  call void @llvm.var.annotation(i8* %9, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str, i64 0, i64 0), i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.1, i64 0, i64 0), i32 17)
  %10 = call float @random()
  store float %10, float* %5, align 4, !tbaa !6
  %11 = bitcast float* %6 to i8*
  call void @llvm.lifetime.start(i64 4, i8* %11) #3
  call void @llvm.var.annotation(i8* %11, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str, i64 0, i64 0), i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.1, i64 0, i64 0), i32 18)
  %12 = icmp eq i32 %0, 0
  %13 = select i1 %12, float 0x3FF3333340000000, float 1.000000e+00
  %14 = icmp ne i32 %2, 0
  %15 = load float, float* %4, align 4
  %16 = load float, float* %5, align 4
  %17 = select i1 %14, float %15, float %16
  %18 = fmul float %13, %17
  %19 = icmp eq i32 %1, 0
  %20 = fpext float %18 to double
  %21 = select i1 %19, double 3.000000e-01, double 8.000000e-01
  %22 = fadd double %21, %20
  %23 = fptrunc double %22 to float
  call void @llvm.lifetime.end(i64 4, i8* %11) #3
  call void @llvm.lifetime.end(i64 4, i8* %9) #3
  call void @llvm.lifetime.end(i64 4, i8* %7) #3
  ret float %23
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
