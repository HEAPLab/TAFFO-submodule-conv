float __attribute((annotate("no_float 39 9"))) global = 3.333;

float fun(float x, float y){
    float local;
    local = x * y + global;
    return local;
}

int funInt(float x, float y){
    int local;
    local = x * y + global;
    return local;
}

int main() {
    float a=10.2049;
    float __attribute((annotate("no_float 34 8"))) b=10.1024;
    int c = 2;
    
    
    a = fun(b,a);
    printf("%f\n",a);
    
    a = fun(a,b);
    printf("%f\n",a);
    
    a = fun(b,b);
    printf("%f\n",a);
    
    a = fun(a,a);
    printf("%f\n",a);
    
    
    
    // ------------------ //
    
    b = a/4000;
    
    b = fun(b,b);
    printf("%f\n",b);
    
    b = fun(a,b);
    printf("%f\n",b);
    
    b = fun(b,a);
    printf("%f\n",b);
    
    b = fun(a,a);
    printf("%f\n",b);
    
        
    // ----------------- //
    
    b = a/4096;
    
    c = fun(b,b);
    printf("%d\n",c);
    
    c = fun(b,b);
    printf("%d\n",c);
    
    c = fun(b,a);
    printf("%d\n",c);
    
    c = fun(a,b);
    printf("%d\n",c);
    
    
    // ------------------ //
    
    
    printf("*******************\n");
    
    b=10.05;

    a = funInt(b,b);
    printf("%f\n",a);
    
    a = funInt(b,a);
    printf("%f\n",a);
    
    a = funInt(a,b);
    printf("%f\n",a);
    
    a = funInt(a,a);
    printf("%f\n",a);
    
    
    a = 3.19;
    
    b = funInt(a,b);
    printf("%f\n",b);
    
    b = funInt(b,a);
    printf("%f\n",b);
    
    b = funInt(a,a);
    printf("%f\n",b);
    
    b = funInt(b,b);
    printf("%f\n",b);
    
    
    
    c = funInt(b,b);
    printf("%d\n",c);
    
    c = funInt(a,b);
    printf("%d\n",c);
    
    c = funInt(a,a);
    printf("%d\n",c);
    
    c = funInt(b,a);
    printf("%d\n",c);
    

    printf("-------------------\n");
    return 0;
}
