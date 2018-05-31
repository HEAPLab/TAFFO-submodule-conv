int main()
{
    float out;
    #pragma clang attribute push( __attribute__((annotate("no_float 16 16"))) , apply_to = variable)
    float a=10;
    float b;
    #pragma clang attribute pop
    
    #pragma clang attribute push( __attribute__((annotate("no_float 8 24"))) , apply_to = variable)
    float c = 2.1024;
    float d;
    #pragma clang attribute pop
    
    b = a * 0.21024;
    b /= 2;
    c /= 2;
    d = b + c;
    
    out = d;
    printf("%f\n",out);
    
    return 0;
}
