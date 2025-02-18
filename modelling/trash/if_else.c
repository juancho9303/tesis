#include <stdio.h>
#include <math.h>

#define N 10
#define R 3
#define a 5.0
#define pi 3.141592
#define M 5000
#define gamma 1.2

double X_s,I_R,s;

void main()
{
  
  for (s = 0.01; s < N; s=s+0.01)
    {
      
      if (s < 1.0)
	{
	  X_s = (1.0 / sqrt(1.0 - s*s)) * log((1.0+(sqrt(1-s*s)))/(s));
	  I_R = (M/(2.0*pi*a*a*gamma*(1.0-s*s)*(1.0-s*s)))*((2.0+s*s)*X_s-3.0);
	  printf("I(R) %6lf\n", I_R);
	}
      else if(s>1.01)
	{
	  X_s = (1.0 / sqrt(s*s - 1.0)) * acos(1/s);
	  I_R = (M/(2.0*pi*a*a*gamma*(1.0-s*s)*(1.0-s*s)))*((2.0+s*s)*X_s-3.0);
	  printf("I(R) %6lf\n", I_R);
	}
      
    }
}
