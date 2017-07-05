

float fpextfptrunc(__attribute((annotate("no_float"))) float a, __attribute((annotate("no_float"))) double b)
{
  __attribute((annotate("no_float"))) double c = 123.0;
  c += a;
  return (float)c + b;
}

