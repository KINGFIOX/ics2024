int main(void) {
  register int i = 1;
  register int sum = 0;
  for (i = 1; i <= 100; i++) {
    sum += i;
  }
  return sum;
}
