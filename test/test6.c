
float test(int a)
{
  __attribute((annotate("no_float"))) float c[10];
  __attribute((annotate("no_float"))) float *b = c;
  b[5] = a;
  return b[5];
}


