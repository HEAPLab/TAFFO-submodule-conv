; ModuleID = '/Users/danielecattaneo/Documents/Sviluppo C/antifloat/test/test8.c'
source_filename = "/Users/danielecattaneo/Documents/Sviluppo C/antifloat/test/test8.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.13.0"

%struct.spell = type { float, i32 }
%struct.chara = type { float, i32, float }

@.str = private unnamed_addr constant [15 x i8] c"force_no_float\00", section "llvm.metadata"
@.str.1 = private unnamed_addr constant [67 x i8] c"/Users/danielecattaneo/Documents/Sviluppo C/antifloat/test/test8.c\00", section "llvm.metadata"

; Function Attrs: noinline nounwind ssp uwtable
define float @cast(%struct.spell* %spell, %struct.chara* %caster, %struct.chara* %enemy) #0 {
entry:
  %spell.addr = alloca %struct.spell*, align 8
  %caster.addr = alloca %struct.chara*, align 8
  %enemy.addr = alloca %struct.chara*, align 8
  %might = alloca float, align 4
  %ehp = alloca float*, align 8
  store %struct.spell* %spell, %struct.spell** %spell.addr, align 8
  store %struct.chara* %caster, %struct.chara** %caster.addr, align 8
  store %struct.chara* %enemy, %struct.chara** %enemy.addr, align 8
  %0 = load %struct.spell*, %struct.spell** %spell.addr, align 8
  %mpcost = getelementptr inbounds %struct.spell, %struct.spell* %0, i32 0, i32 1
  %1 = load i32, i32* %mpcost, align 4
  %2 = load %struct.chara*, %struct.chara** %caster.addr, align 8
  %mp = getelementptr inbounds %struct.chara, %struct.chara* %2, i32 0, i32 1
  %3 = load i32, i32* %mp, align 4
  %sub = sub nsw i32 %3, %1
  store i32 %sub, i32* %mp, align 4
  %might1 = bitcast float* %might to i8*
  call void @llvm.var.annotation(i8* %might1, i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([67 x i8], [67 x i8]* @.str.1, i32 0, i32 0), i32 18)
  %4 = load %struct.spell*, %struct.spell** %spell.addr, align 8
  %might2 = getelementptr inbounds %struct.spell, %struct.spell* %4, i32 0, i32 0
  %5 = load float, float* %might2, align 4
  store float %5, float* %might, align 4
  %ehp3 = bitcast float** %ehp to i8*
  call void @llvm.var.annotation(i8* %ehp3, i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([67 x i8], [67 x i8]* @.str.1, i32 0, i32 0), i32 19)
  %6 = load %struct.chara*, %struct.chara** %enemy.addr, align 8
  %hp = getelementptr inbounds %struct.chara, %struct.chara* %6, i32 0, i32 0
  store float* %hp, float** %ehp, align 8
  %7 = load float, float* %might, align 4
  %8 = load %struct.chara*, %struct.chara** %enemy.addr, align 8
  %mdef = getelementptr inbounds %struct.chara, %struct.chara* %8, i32 0, i32 2
  %9 = load float, float* %mdef, align 4
  %sub4 = fsub float %7, %9
  %10 = load float*, float** %ehp, align 8
  %11 = load float, float* %10, align 4
  %sub5 = fsub float %11, %sub4
  store float %sub5, float* %10, align 4
  %12 = load float*, float** %ehp, align 8
  %13 = load float, float* %12, align 4
  ret float %13
}

; Function Attrs: nounwind
declare void @llvm.var.annotation(i8*, i8*, i8*, i32) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"PIC Level", i32 2}
!1 = !{!"clang version 4.0.0 (tags/RELEASE_400/final)"}
