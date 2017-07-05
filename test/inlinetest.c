


float hello(__attribute__((annotate("no_float"))) float abc) __attribute__((always_inline)) {
  return abc + (float)5.0;
}


int main(int argc, char *argv[]) {
	__attribute__((annotate("no_float"))) float test = 123.0;
	test = hello(test);
	printf("%f\n", test);
	return 0;
}

