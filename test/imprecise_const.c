
int main(int argc, char *argv[])
{
  __attribute__((annotate("no_float"))) double magic = 1.234567890123456789;
  return magic + 2.3456778912345678;
}

