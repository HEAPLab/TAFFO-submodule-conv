


float example(float a, float b, float c, float d)
{
  float x = 1.0;
  float y __attribute((annotate("no_float"))) = 2.0;
  float z =3.0;

  return (x * (y / z)) * x;
}


int main(int argc, char *argv[])
{
  example(1, 2, 3, 4);
}

