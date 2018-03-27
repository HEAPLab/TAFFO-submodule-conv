
float oven(int stuff, int baked, float cherry)
{
  float __attribute__((annotate("no_float"))) cake = baked + stuff;
  return cake + cherry;
}


