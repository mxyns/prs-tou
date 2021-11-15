#include <stdlib.h>

int drop_message(double random_val_0_20d, double *param_2, int param_3)

{
  int rnd;
  double p;
  
  if (param_3 % 10 == 0) {
    rnd = rand();
    *param_2 = ((double)(rnd % 100) / 100.0) * 20.0;
  }
  
  p = 0.05;
  if (*param_2 <= random_val_0_20d) {
    p = 0.95;
  }

  rnd = rand();
  return (double)(rnd % 100) / 100.0 < p;
}


int main() {
    int result = rand();
    double random_0_20d = ((double)(result % 100) / 100.0) * 20.0;
    double param2 = 0;
    int param3 = 0;

    result = drop_message(random_0_20d,uStack151616);
}