
float test(int a)
{
  float c[10];
  __attribute((annotate("force_no_float"))) float *b = c;
  b[5] = a;
  return b[5];
}


