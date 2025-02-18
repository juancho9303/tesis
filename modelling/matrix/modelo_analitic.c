#include <stdio.h>
#include <math.h>
#include <gsl/gsl_integration.h>

// SE UTILIZA EL MODELO DE HERNQUIST PARA ENCONTRAR LA FORMA DE LA DISPERSION DE VELOCIDADES PROYECTADA Y PODER AJUSTAR LOS PARAMETROS QUE MEJOR SE AJUSTEN A LAS OBSERVACIONES

#define K 10.0
#define N 5
#define M 10e-3
#define GAMMA 1.2
#define G 43007.1

struct param{
  double beta;  
};

double R;
int i,j,k,l;

// ACA SE DEFINE LA INTEGRAL QUE CONTIENE TODOS LOS TERMINOS DE LAS CONTRIBUCIONES DE MASA ESTELAR Y MATERIA OSCURA 

double integrando (double r, double a_s[N], double M_s[N], double a_dm[N], double M_dm[N], void * params) 
{
  struct param parameters = *(struct param *) params;
  
  //double a_s[i] = parameters.a_s;
  //double a_dm[j] = parameters.a_dm;
  //double M_s[k] = parameters.M_s;
  //double M_dm[l] = parameters.M_dm;
  double beta = parameters.beta;
  double alpha, a1, a2, a3, a4, a5, b1, b2, b3, b4, b5;
  double M_sM_dm, M_dmM_s, M_sM_s, M_dmM_dm;
  
  alpha = ((1.0 - beta*((R*R)/(r*r)))*((r)/(sqrt(r*r-R*R))));
  
  M_sM_s = M_s[k]*M_s[k]*a_s[i]*(((12.0*pow((a_s[i]+r),4.0)*log((a_s[i]+r)/(r))) - a_s[i]*(25.0*a_s[i]*a_s[i]*a_s[i]+52.0*a_s[i]*a_s[i]*r+42.0*a_s[i]*r*r+12.0*r*r*r))/(12.0*pow(a_s[i],5)*pow((a_s[i]+r),4)));
  
  M_dmM_dm = M_dm[l]*M_dm[l]*a_dm[j]*(((12.0*pow((a_dm[j]+r),4.0)*log((a_dm[j]+r)/(r))) - a_dm[j]*(25.0*a_dm[j]*a_dm[j]*a_dm[j]+52.0*a_dm[j]*a_dm[j]*r+42.0*a_dm[j]*r*r+12.0*r*r*r))/(12.0*pow(a_dm[j],5)*pow((a_dm[j]+r),4)));
  
  //----------------------------------------------------------------
  // Paso a paso sin hacer reducciones para los terminos cruzados.
  //--------------------------------------------------------------
  
  a1 = 2.0*pow(a_s[i],3.0)*pow((a_s[i]-a_dm[j]),4.0)*a_dm[j]*a_dm[j]*(a_s[i]+r)*(a_s[i]+r)*(a_dm[j]+r);
  a2 = 2.0*pow((a_s[i]-a_dm[j]),4.0)*(a_s[i]+r)*(a_s[i]+r)*(a_dm[j]+r)*log(r);
  a3 = 2.0*a_dm[j]*a_dm[j]*(6.0*a_s[i]*a_s[i]-4.0*a_s[i]*a_dm[j]+a_dm[j]*a_dm[j])*(a_s[i]+r)*(a_s[i]+r)*(a_dm[j]+r)*log(a_s[i]+r);
  a4 = 2.0*pow(a_s[i],4.0)+4.0*pow(a_s[i],3.0)*r-2.0*a_dm[j]*a_dm[j]*r*(a_dm[j]+r)+3.0*a_s[i]*a_dm[j]*(-a_dm[j]*a_dm[j]+a_dm[j]*r+2.0*r*r)+a_s[i]*a_s[i]*(7.0*a_dm[j]*a_dm[j]+7.0*a_dm[j]*r+2.0*r*r);
  a5 = 2.0*a_s[i]*a_s[i]*(a_s[i]-4.0*a_dm[j])*(a_s[i]+r)*(a_s[i]+r)*(a_dm[j]+r)*log(a_dm[j]+r);
  
  M_sM_dm = M_s[k]*M_dm[l]*((-1.0)/(a1))*(a2-a3+a_s[i]*((a_s[i]-a_dm[j])*a_dm[j]*a4-a5));
  //printf ("M_sM_dm  = %6lf\t % .18f\n", r, M_sM_dm);
  
  b1 = 2.0*pow(a_dm[j],3)*pow((a_dm[j]-a_s[i]),4.0)*a_s[i]*a_s[i]*(a_dm[j]+r)*(a_dm[j]+r)*(a_s[i]+r);
  b2 = 2.0*pow((a_dm[j]-a_s[i]),4.0)*(a_dm[j]+r)*(a_dm[j]+r)*(a_s[i]+r)*log(r);
  b3 = 2.0*a_s[i]*a_s[i]*(6.0*a_dm[j]*a_dm[j]-4.0*a_dm[j]*a_s[i]+a_s[i]*a_s[i])*(a_dm[j]+r)*(a_dm[j]+r)*(a_s[i]+r)*log(a_dm[j]+r);
  b4 = 2.0*pow(a_dm[j],4.0)+4.0*pow(a_dm[j],3.0)*r-2.0*a_s[i]*a_s[i]*r*(a_s[i]+r)+3.0*a_dm[j]*a_s[i]*(-a_s[i]*a_s[i]+a_s[i]*r+2.0*r*r)+a_dm[j]*a_dm[j]*(7.0*a_s[i]*a_s[i]+7.0*a_s[i]*r+2.0*r*r);
  b5 = 2.0*a_dm[j]*a_dm[j]*(a_dm[j]-4.0*a_s[i])*(a_dm[j]+r)*(a_dm[j]+r)*(a_s[i]+r)*log(a_s[i]+r);
  
  M_dmM_s = M_dm[l]*M_s[k]*((-1.0)/(b1))*(b2-b3+a_dm[j]*((a_dm[j]-a_s[i])*a_s[i]*b4-b5));
  //printf ("M_dmM_s  = %6lf\t % .18f\n", r, M_dmM_s);
  
  double integrando = alpha*(M_sM_s + M_dmM_dm + M_sM_dm + M_dmM_s);
  
  return integrando;
}

int main(){
  
  FILE *mod,*script;
  int warn;
  int Nint = 1000; // Numero de intervalos
  double X_s, I_R, s, R, sig_p, rho, re1, re2, result1, error1, result2, error2, a_dm[N], a_s[N], M_s[N], M_dm[N], beta;
  struct param params; // structura de parametros
  gsl_integration_workspace *z = gsl_integration_workspace_alloc(Nint);
  gsl_function F1;
  
  // PARAMETROS
  
  a_dm[0] = 1.1;
  a_dm[1] = 1.6;
  a_dm[2] = 2.3;
  a_dm[3] = 2.7;
  a_dm[4] = 3.2;
  a_dm[5] = 3.4;

  a_s[0] = 1.2;
  a_s[1] = 1.9;
  a_s[2] = 2.3;
  a_s[3] = 2.7;
  a_s[4] = 3.3;
  a_s[5] = 3.6;

  M_dm[0] = 2.7;
  M_dm[1] = 2.9;
  M_dm[2] = 3.2;
  M_dm[3] = 3.8;
  M_dm[4] = 4.1;
  M_dm[5] = 4.3;

  M_s[0] = 2.4;
  M_s[1] = 2.8;
  M_s[2] = 3.2;
  M_s[3] = 3.7;
  M_s[4] = 3.9;
  M_s[5] = 4.1;
  
  for(i = 0; i < N;  i++)
    {
      for(j = 0; j < N;  j++)
	{
	  for(k = 0; k < N; k++)
	    {
	      for(l = 0; l < N; l++)
		{
		  
		  beta = 2.0;
		  
		  params.beta     =  beta;
		  //params.a_s   =  a_s[N];
		  //params.a_dm  =  a_dm[N];
		  //params.M_s   =  M_s[N];
		  //params.M_dm  =  M_dm[N];
		  
		  mod=fopen("model.dat","w");
		  
		  for(R = 0.01; R < K; R=R+0.01)
		    {
		      
		      F1.function = &integrando;
		      F1.params = &params;
		      
		      gsl_integration_qags(&F1, R, 40, 0, 1e-2, Nint, z, &result1, &error1); 
		    //printf ("result  = % .18f\n", result1);
		      
		      re1 = result1;
		      s = R/a_s[i];
		      
		      if (R < 1.0-1.0e-9)
			{      	
		X_s = (1.0 / sqrt(1.0 - s*s)) * log((1.0+(sqrt(1.0-s*s)))/(s));
		//printf("sig_p^2(R) %6lf\n", sig_p);	
			}
		      
		      if(R >= 1.0-1.0e-10 && R <= 1.0+1.0e-10)
			{
			  X_s = 1.0;
			}
		      
		      if (R > 1.0+1.0e-9)
			{	       	  
			  X_s = (1.0 / sqrt(s*s - 1.0)) * acos(1.0/s);
			  //printf("sig_p^2(R) %6lf\n", sig_p);
			}      
		      
		      I_R = (M_s[k]/(2.0*M_PI*a_s[i]*a_s[i]*GAMMA*(1.0-s*s)*(1.0-s*s)))*((2.0+s*s)*X_s-3.0);
		      sig_p = ((2.0*G*M_s[k]*M_s[k]*a_s[i])/(GAMMA*(I_R)*2.0*M_PI))*re1;
		      //printf("%16.8e\t %16.8e\n", R, I_R);
		      fprintf(mod,"%16.8e\t %16.8e\n", R, sig_p);
		    }
		  
		  gsl_integration_workspace_free(z);
		  
		  fclose(mod);
   
		  return(warn);
		}
	    }
	}      
    }  
}
