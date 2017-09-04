
void test(void)
{
	double a __attribute((annotate("no_float")));
	double b = 98.0, c = 77.0;
	b = b - c;
	a = b * 2.0;
}

int main(int argc, char *argv[])
{
	test();
	return 0;
}


