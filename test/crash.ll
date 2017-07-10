; ModuleID = 'linear-algebra/kernels/atax/atax.c'
source_filename = "linear-algebra/kernels/atax/atax.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

%struct.__sFILE = type { i8*, i32, i32, i16, i16, %struct.__sbuf, i32, i8*, i32 (i8*)*, i32 (i8*, i8*, i32)*, i64 (i8*, i64, i32)*, i32 (i8*, i8*, i32)*, %struct.__sbuf, %struct.__sFILEX*, i32, [3 x i8], [1 x i8], %struct.__sbuf, i32, i64 }
%struct.__sFILEX = type opaque
%struct.__sbuf = type { i8*, i32 }

@.str = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@__stderrp = external global %struct.__sFILE*, align 8
@.str.1 = private unnamed_addr constant [8 x i8] c"%0.2lf \00", align 1
@.str.2 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main(i32 %argc, i8** %argv) #0 {
entry:
  %retval = alloca i32, align 4
  %argc.addr = alloca i32, align 4
  %argv.addr = alloca i8**, align 8
  %nx = alloca i32, align 4
  %ny = alloca i32, align 4
  %A = alloca [4000 x [4000 x double]]*, align 8
  %x = alloca [4000 x double]*, align 8
  %y = alloca [4000 x double]*, align 8
  %tmp = alloca [4000 x double]*, align 8
  store i32 0, i32* %retval, align 4
  store i32 %argc, i32* %argc.addr, align 4
  store i8** %argv, i8*** %argv.addr, align 8
  store i32 4000, i32* %nx, align 4
  store i32 4000, i32* %ny, align 4
  %call = call i8* @polybench_alloc_data(i32 16000000, i32 8)
  %0 = bitcast i8* %call to [4000 x [4000 x double]]*
  store [4000 x [4000 x double]]* %0, [4000 x [4000 x double]]** %A, align 8
  %call1 = call i8* @polybench_alloc_data(i32 4000, i32 8)
  %1 = bitcast i8* %call1 to [4000 x double]*
  store [4000 x double]* %1, [4000 x double]** %x, align 8
  %call2 = call i8* @polybench_alloc_data(i32 4000, i32 8)
  %2 = bitcast i8* %call2 to [4000 x double]*
  store [4000 x double]* %2, [4000 x double]** %y, align 8
  %call3 = call i8* @polybench_alloc_data(i32 4000, i32 8)
  %3 = bitcast i8* %call3 to [4000 x double]*
  store [4000 x double]* %3, [4000 x double]** %tmp, align 8
  %4 = load i32, i32* %nx, align 4
  %5 = load i32, i32* %ny, align 4
  %6 = load [4000 x [4000 x double]]*, [4000 x [4000 x double]]** %A, align 8
  %arraydecay = getelementptr inbounds [4000 x [4000 x double]], [4000 x [4000 x double]]* %6, i32 0, i32 0
  %7 = load [4000 x double]*, [4000 x double]** %x, align 8
  %arraydecay4 = getelementptr inbounds [4000 x double], [4000 x double]* %7, i32 0, i32 0
  call void @init_array(i32 %4, i32 %5, [4000 x double]* %arraydecay, double* %arraydecay4)
  call void (...) @polybench_timer_start()
  %8 = load i32, i32* %nx, align 4
  %9 = load i32, i32* %ny, align 4
  %10 = load [4000 x [4000 x double]]*, [4000 x [4000 x double]]** %A, align 8
  %arraydecay5 = getelementptr inbounds [4000 x [4000 x double]], [4000 x [4000 x double]]* %10, i32 0, i32 0
  %11 = load [4000 x double]*, [4000 x double]** %x, align 8
  %arraydecay6 = getelementptr inbounds [4000 x double], [4000 x double]* %11, i32 0, i32 0
  %12 = load [4000 x double]*, [4000 x double]** %y, align 8
  %arraydecay7 = getelementptr inbounds [4000 x double], [4000 x double]* %12, i32 0, i32 0
  %13 = load [4000 x double]*, [4000 x double]** %tmp, align 8
  %arraydecay8 = getelementptr inbounds [4000 x double], [4000 x double]* %13, i32 0, i32 0
  call void @kernel_atax(i32 %8, i32 %9, [4000 x double]* %arraydecay5, double* %arraydecay6, double* %arraydecay7, double* %arraydecay8)
  call void (...) @polybench_timer_stop()
  call void (...) @polybench_timer_print()
  %14 = load i32, i32* %argc.addr, align 4
  %cmp = icmp sgt i32 %14, 42
  br i1 %cmp, label %land.lhs.true, label %if.end

land.lhs.true:                                    ; preds = %entry
  %15 = load i8**, i8*** %argv.addr, align 8
  %arrayidx = getelementptr inbounds i8*, i8** %15, i64 0
  %16 = load i8*, i8** %arrayidx, align 8
  %call9 = call i32 @strcmp(i8* %16, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str, i32 0, i32 0))
  %tobool = icmp ne i32 %call9, 0
  br i1 %tobool, label %if.end, label %if.then

if.then:                                          ; preds = %land.lhs.true
  %17 = load i32, i32* %nx, align 4
  %18 = load [4000 x double]*, [4000 x double]** %y, align 8
  %arraydecay10 = getelementptr inbounds [4000 x double], [4000 x double]* %18, i32 0, i32 0
  call void @print_array(i32 %17, double* %arraydecay10)
  br label %if.end

if.end:                                           ; preds = %if.then, %land.lhs.true, %entry
  %19 = load [4000 x [4000 x double]]*, [4000 x [4000 x double]]** %A, align 8
  %20 = bitcast [4000 x [4000 x double]]* %19 to i8*
  %call11 = call i32 (i8*, ...) bitcast (i32 (...)* @free to i32 (i8*, ...)*)(i8* %20)
  %21 = load [4000 x double]*, [4000 x double]** %x, align 8
  %22 = bitcast [4000 x double]* %21 to i8*
  %call12 = call i32 (i8*, ...) bitcast (i32 (...)* @free to i32 (i8*, ...)*)(i8* %22)
  %23 = load [4000 x double]*, [4000 x double]** %y, align 8
  %24 = bitcast [4000 x double]* %23 to i8*
  %call13 = call i32 (i8*, ...) bitcast (i32 (...)* @free to i32 (i8*, ...)*)(i8* %24)
  %25 = load [4000 x double]*, [4000 x double]** %tmp, align 8
  %26 = bitcast [4000 x double]* %25 to i8*
  %call14 = call i32 (i8*, ...) bitcast (i32 (...)* @free to i32 (i8*, ...)*)(i8* %26)
  ret i32 0
}

declare i8* @polybench_alloc_data(i32, i32) #1

; Function Attrs: noinline nounwind ssp uwtable
define internal void @init_array(i32 %nx, i32 %ny, [4000 x double]* %A, double* %x) #0 {
entry:
  %nx.addr = alloca i32, align 4
  %ny.addr = alloca i32, align 4
  %A.addr = alloca [4000 x double]*, align 8
  %x.addr = alloca double*, align 8
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  store i32 %nx, i32* %nx.addr, align 4
  store i32 %ny, i32* %ny.addr, align 4
  store [4000 x double]* %A, [4000 x double]** %A.addr, align 8
  store double* %x, double** %x.addr, align 8
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, i32* %i, align 4
  %1 = load i32, i32* %ny.addr, align 4
  %cmp = icmp slt i32 %0, %1
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %2 = load i32, i32* %i, align 4
  %conv = sitofp i32 %2 to double
  %mul = fmul double %conv, 0x400921FB54442D18
  %3 = load double*, double** %x.addr, align 8
  %4 = load i32, i32* %i, align 4
  %idxprom = sext i32 %4 to i64
  %arrayidx = getelementptr inbounds double, double* %3, i64 %idxprom
  store double %mul, double* %arrayidx, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %5 = load i32, i32* %i, align 4
  %inc = add nsw i32 %5, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  store i32 0, i32* %i, align 4
  br label %for.cond1

for.cond1:                                        ; preds = %for.inc20, %for.end
  %6 = load i32, i32* %i, align 4
  %7 = load i32, i32* %nx.addr, align 4
  %cmp2 = icmp slt i32 %6, %7
  br i1 %cmp2, label %for.body4, label %for.end22

for.body4:                                        ; preds = %for.cond1
  store i32 0, i32* %j, align 4
  br label %for.cond5

for.cond5:                                        ; preds = %for.inc17, %for.body4
  %8 = load i32, i32* %j, align 4
  %9 = load i32, i32* %ny.addr, align 4
  %cmp6 = icmp slt i32 %8, %9
  br i1 %cmp6, label %for.body8, label %for.end19

for.body8:                                        ; preds = %for.cond5
  %10 = load i32, i32* %i, align 4
  %conv9 = sitofp i32 %10 to double
  %11 = load i32, i32* %j, align 4
  %add = add nsw i32 %11, 1
  %conv10 = sitofp i32 %add to double
  %mul11 = fmul double %conv9, %conv10
  %12 = load i32, i32* %nx.addr, align 4
  %conv12 = sitofp i32 %12 to double
  %div = fdiv double %mul11, %conv12
  %13 = load [4000 x double]*, [4000 x double]** %A.addr, align 8
  %14 = load i32, i32* %i, align 4
  %idxprom13 = sext i32 %14 to i64
  %arrayidx14 = getelementptr inbounds [4000 x double], [4000 x double]* %13, i64 %idxprom13
  %15 = load i32, i32* %j, align 4
  %idxprom15 = sext i32 %15 to i64
  %arrayidx16 = getelementptr inbounds [4000 x double], [4000 x double]* %arrayidx14, i64 0, i64 %idxprom15
  store double %div, double* %arrayidx16, align 8
  br label %for.inc17

for.inc17:                                        ; preds = %for.body8
  %16 = load i32, i32* %j, align 4
  %inc18 = add nsw i32 %16, 1
  store i32 %inc18, i32* %j, align 4
  br label %for.cond5

for.end19:                                        ; preds = %for.cond5
  br label %for.inc20

for.inc20:                                        ; preds = %for.end19
  %17 = load i32, i32* %i, align 4
  %inc21 = add nsw i32 %17, 1
  store i32 %inc21, i32* %i, align 4
  br label %for.cond1

for.end22:                                        ; preds = %for.cond1
  ret void
}

declare void @polybench_timer_start(...) #1

; Function Attrs: noinline nounwind ssp uwtable
define internal void @kernel_atax(i32 %nx, i32 %ny, [4000 x double]* %A, double* %x, double* %y, double* %tmp) #0 {
entry:
  %nx.addr = alloca i32, align 4
  %ny.addr = alloca i32, align 4
  %A.addr = alloca [4000 x double]*, align 8
  %x.addr = alloca double*, align 8
  %y.addr = alloca double*, align 8
  %tmp.addr = alloca double*, align 8
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  store i32 %nx, i32* %nx.addr, align 4
  store i32 %ny, i32* %ny.addr, align 4
  store [4000 x double]* %A, [4000 x double]** %A.addr, align 8
  store double* %x, double** %x.addr, align 8
  store double* %y, double** %y.addr, align 8
  store double* %tmp, double** %tmp.addr, align 8
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, i32* %i, align 4
  %1 = load i32, i32* %nx.addr, align 4
  %cmp = icmp slt i32 %0, %1
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %2 = load double*, double** %y.addr, align 8
  %3 = load i32, i32* %i, align 4
  %idxprom = sext i32 %3 to i64
  %arrayidx = getelementptr inbounds double, double* %2, i64 %idxprom
  store double 0.000000e+00, double* %arrayidx, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %4 = load i32, i32* %i, align 4
  %inc = add nsw i32 %4, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  store i32 0, i32* %i, align 4
  br label %for.cond3

for.cond3:                                        ; preds = %for.inc42, %for.end
  %5 = load i32, i32* %i, align 4
  %6 = load i32, i32* %ny.addr, align 4
  %cmp4 = icmp slt i32 %5, %6
  br i1 %cmp4, label %for.body5, label %for.end44

for.body5:                                        ; preds = %for.cond3
  %7 = load double*, double** %tmp.addr, align 8
  %8 = load i32, i32* %i, align 4
  %idxprom6 = sext i32 %8 to i64
  %arrayidx7 = getelementptr inbounds double, double* %7, i64 %idxprom6
  store double 0.000000e+00, double* %arrayidx7, align 8
  store i32 0, i32* %j, align 4
  br label %for.cond8

for.cond8:                                        ; preds = %for.inc21, %for.body5
  %9 = load i32, i32* %j, align 4
  %10 = load i32, i32* %ny.addr, align 4
  %cmp9 = icmp slt i32 %9, %10
  br i1 %cmp9, label %for.body10, label %for.end23

for.body10:                                       ; preds = %for.cond8
  %11 = load double*, double** %tmp.addr, align 8
  %12 = load i32, i32* %i, align 4
  %idxprom11 = sext i32 %12 to i64
  %arrayidx12 = getelementptr inbounds double, double* %11, i64 %idxprom11
  %13 = load double, double* %arrayidx12, align 8
  %14 = load [4000 x double]*, [4000 x double]** %A.addr, align 8
  %15 = load i32, i32* %i, align 4
  %idxprom13 = sext i32 %15 to i64
  %arrayidx14 = getelementptr inbounds [4000 x double], [4000 x double]* %14, i64 %idxprom13
  %16 = load i32, i32* %j, align 4
  %idxprom15 = sext i32 %16 to i64
  %arrayidx16 = getelementptr inbounds [4000 x double], [4000 x double]* %arrayidx14, i64 0, i64 %idxprom15
  %17 = load double, double* %arrayidx16, align 8
  %18 = load double*, double** %x.addr, align 8
  %19 = load i32, i32* %j, align 4
  %idxprom17 = sext i32 %19 to i64
  %arrayidx18 = getelementptr inbounds double, double* %18, i64 %idxprom17
  %20 = load double, double* %arrayidx18, align 8
  %mul = fmul double %17, %20
  %add = fadd double %13, %mul
  %21 = load double*, double** %tmp.addr, align 8
  %22 = load i32, i32* %i, align 4
  %idxprom19 = sext i32 %22 to i64
  %arrayidx20 = getelementptr inbounds double, double* %21, i64 %idxprom19
  store double %add, double* %arrayidx20, align 8
  br label %for.inc21

for.inc21:                                        ; preds = %for.body10
  %23 = load i32, i32* %j, align 4
  %inc22 = add nsw i32 %23, 1
  store i32 %inc22, i32* %j, align 4
  br label %for.cond8

for.end23:                                        ; preds = %for.cond8
  store i32 0, i32* %j, align 4
  br label %for.cond24

for.cond24:                                       ; preds = %for.inc39, %for.end23
  %24 = load i32, i32* %j, align 4
  %25 = load i32, i32* %ny.addr, align 4
  %cmp25 = icmp slt i32 %24, %25
  br i1 %cmp25, label %for.body26, label %for.end41

for.body26:                                       ; preds = %for.cond24
  %26 = load double*, double** %y.addr, align 8
  %27 = load i32, i32* %j, align 4
  %idxprom27 = sext i32 %27 to i64
  %arrayidx28 = getelementptr inbounds double, double* %26, i64 %idxprom27
  %28 = load double, double* %arrayidx28, align 8
  %29 = load [4000 x double]*, [4000 x double]** %A.addr, align 8
  %30 = load i32, i32* %i, align 4
  %idxprom29 = sext i32 %30 to i64
  %arrayidx30 = getelementptr inbounds [4000 x double], [4000 x double]* %29, i64 %idxprom29
  %31 = load i32, i32* %j, align 4
  %idxprom31 = sext i32 %31 to i64
  %arrayidx32 = getelementptr inbounds [4000 x double], [4000 x double]* %arrayidx30, i64 0, i64 %idxprom31
  %32 = load double, double* %arrayidx32, align 8
  %33 = load double*, double** %tmp.addr, align 8
  %34 = load i32, i32* %i, align 4
  %idxprom33 = sext i32 %34 to i64
  %arrayidx34 = getelementptr inbounds double, double* %33, i64 %idxprom33
  %35 = load double, double* %arrayidx34, align 8
  %mul35 = fmul double %32, %35
  %add36 = fadd double %28, %mul35
  %36 = load double*, double** %y.addr, align 8
  %37 = load i32, i32* %j, align 4
  %idxprom37 = sext i32 %37 to i64
  %arrayidx38 = getelementptr inbounds double, double* %36, i64 %idxprom37
  store double %add36, double* %arrayidx38, align 8
  br label %for.inc39

for.inc39:                                        ; preds = %for.body26
  %38 = load i32, i32* %j, align 4
  %inc40 = add nsw i32 %38, 1
  store i32 %inc40, i32* %j, align 4
  br label %for.cond24

for.end41:                                        ; preds = %for.cond24
  br label %for.inc42

for.inc42:                                        ; preds = %for.end41
  %39 = load i32, i32* %i, align 4
  %inc43 = add nsw i32 %39, 1
  store i32 %inc43, i32* %i, align 4
  br label %for.cond3

for.end44:                                        ; preds = %for.cond3
  ret void
}

declare void @polybench_timer_stop(...) #1

declare void @polybench_timer_print(...) #1

declare i32 @strcmp(i8*, i8*) #1

; Function Attrs: noinline nounwind ssp uwtable
define internal void @print_array(i32 %nx, double* %y) #0 {
entry:
  %nx.addr = alloca i32, align 4
  %y.addr = alloca double*, align 8
  %i = alloca i32, align 4
  store i32 %nx, i32* %nx.addr, align 4
  store double* %y, double** %y.addr, align 8
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, i32* %i, align 4
  %1 = load i32, i32* %nx.addr, align 4
  %cmp = icmp slt i32 %0, %1
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %2 = load %struct.__sFILE*, %struct.__sFILE** @__stderrp, align 8
  %3 = load double*, double** %y.addr, align 8
  %4 = load i32, i32* %i, align 4
  %idxprom = sext i32 %4 to i64
  %arrayidx = getelementptr inbounds double, double* %3, i64 %idxprom
  %5 = load double, double* %arrayidx, align 8
  %call = call i32 (%struct.__sFILE*, i8*, ...) @fprintf(%struct.__sFILE* %2, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.1, i32 0, i32 0), double %5)
  %6 = load i32, i32* %i, align 4
  %rem = srem i32 %6, 20
  %cmp1 = icmp eq i32 %rem, 0
  br i1 %cmp1, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %7 = load %struct.__sFILE*, %struct.__sFILE** @__stderrp, align 8
  %call2 = call i32 (%struct.__sFILE*, i8*, ...) @fprintf(%struct.__sFILE* %7, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @.str.2, i32 0, i32 0))
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %8 = load i32, i32* %i, align 4
  %inc = add nsw i32 %8, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %9 = load %struct.__sFILE*, %struct.__sFILE** @__stderrp, align 8
  %call3 = call i32 (%struct.__sFILE*, i8*, ...) @fprintf(%struct.__sFILE* %9, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @.str.2, i32 0, i32 0))
  ret void
}

declare i32 @free(...) #1

declare i32 @fprintf(%struct.__sFILE*, i8*, ...) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"PIC Level", i32 2}
!1 = !{!"clang version 4.0.0 (tags/RELEASE_400/final)"}
