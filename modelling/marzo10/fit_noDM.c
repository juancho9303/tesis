#include <stdio.h>
#include <math.h>
#include <gsl/gsl_integration.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_spline.h>

/*
  SE UTILIZA UN MODELO DE HERNQUIST MODIFICADO PARA ENCONTRAR LA FORMA DE LA DISPERSION DE VELOCIDADES PROYECTADA Y PODER AJUSTAR LOS PARAMETROS QUE MEJOR SE AJUSTEN A LAS OBSERVACIONES   
*/

#define K 90                 //90 parsecs que esta por encima de los ~60pc de omega centauri
#define G 43007.1            //unidades de Gadget
#define EPSILON 1            //para suavizar denominadores para evitar indeterminaciones
#define DISTANCE 4.80839
#define LIMIT_RADIUS 0.5
#define ZERO 1.9e-10

// GENERO LA ESTRUCTURA DE PARAMETROS QUE VOY A AJUSTAR
struct param
{
  double beta, a, M, R;  
};

#include "routines_noDM.c"

int evaluate_integral(double R, double beta, double gamma, double a, double M, double *sig_p, double *R_arcmin)
{
  
  int Nint = 1000; // Numero de intervalos
  double result, error, s, aux, X_s, I_R;
  
  gsl_integration_workspace *z = gsl_integration_workspace_alloc(Nint);
  gsl_function F1;
  
  struct param params;
  
  params.beta  =  beta;
  params.a     =  a;
  params.M     =  M;
  params.R     =  R;
  
  F1.function = &integrando;
  F1.params = &params;
  
  gsl_integration_qags(&F1, R, LIMIT_RADIUS, 0, 1e-4, Nint, z, &result, &error); 
  
  s = R/a;
  
  if (s < (1.0-ZERO))
    { 
      aux = 1.0 - s*s;
      X_s = (1.0 / sqrt(aux)) * log((1.0 + sqrt(aux))/s);
      I_R = (M/(2.0*M_PI*a*a*gamma*aux*aux))*( (2.0+s*s)*X_s - 3.0 );
    }
  
  if(s >= (1.0-ZERO) && s <= (1.0+ZERO) )
    {
      X_s = 1.0;
      I_R = (2.0*M)/(15.0*M_PI*a*a*gamma);
    }
  
  if (s > (1.0 + ZERO))
    {
      X_s = (1.0 / sqrt(s*s - 1)) * acos(1.0/s);
      I_R = (M/(2.0*M_PI*a*a*gamma*(1.0-s*s)*(1.0-s*s)))*((2.0+s*s)*X_s - 3.0);
    }      
  
  // SE HACEN LOS CALCULOS UNA VEZ SE TIENE EL VALOR DE LA INTEGRAL
  
  *sig_p = sqrt( (G*result) / (gamma*I_R*M_PI) ) ;
  
  //CONVIERTO DE KILOPARSECS A MINUTOS DE ARCO
  *R_arcmin = R*3437.75/DISTANCE;
  
  gsl_integration_workspace_free(z);
  
  return 0;  
}

// AHORA INICIO EL PROGRAMA PRINCIPAL QUE VA A HACER TODOS LOS CALCULOS PARA ENCONTRAR LA DSPERSION DE VELOCIDADES PROYECTADA

int main(int argc, char *argv[])
{
  
  int warn, i, j, nread, NUMBER_OF_SIGMA_BINS;
  
  double X_s, I_R, s, R[90], R_arcmin[90], sig_p[90], a, M, beta;
  double R_o[12], sig_p_o[12], numb_error[12], chi2, a_min, M_min, beta_min, sig_p_i;
  double chi_min = 1000000.0;
  double gamma, gamma_min, aux;
  char *infile;
  FILE *script,*sigma,*chi_cua;
  
  double gamma_ini, gamma_end, Delta_gamma;
  double a_ini, a_end, Delta_a;
  double M_ini, M_end, Delta_M;
  double beta_ini, beta_end, Delta_beta;
  
  
  infile = argv[1];                          // input file with velocity dispersion bins!
  NUMBER_OF_SIGMA_BINS = atoi(argv[2]);      // eventually 12!
  
  if(argc != 3)
    {
      printf(" Execute as ./exec <infile> <number of lines>\n");
      exit(0);
    }
  
  sigma=fopen(infile,"r"); 
  for(j=0.0; j<NUMBER_OF_SIGMA_BINS; j++)
    nread = fscanf(sigma,"%lf %lf %lf", &R_o[j], &sig_p_o[j], &numb_error[j]);
  fclose(sigma);
  
  gamma_ini  = 0.1;        gamma_end  = 3.0;      Delta_gamma  = 0.5;
  a_ini      = 0.002;      a_end      = 0.06;     Delta_a      = 0.01;
  M_ini      = 0.00001;    M_end      = 0.0008;   Delta_M      = 0.0001;
  beta_ini   = 0.00001;    beta_end   = 1.0;      Delta_beta   = 0.1;
  
  // HAGO LAS VARIACIONES DE LOS PARAMETROS
  
  int Ntotal_iterations, counter, cuenta;
  Ntotal_iterations = (int) ( ((beta_end-beta_ini)/Delta_beta)*((gamma_end-gamma_ini)/Delta_gamma)*((a_end-a_ini)/Delta_a)*((M_end-M_ini)/Delta_M) );
  
  printf("  Running %d iterations in parameter space\n", Ntotal_iterations);
  
  counter=0;
  cuenta=0;
  
  /*DEFINICION DEL VECTOR R[i]*/
  double ii = 0.0;
  for(i=0;i<K;i++)
    {
      R[i] = (ii+1.0)/1000.0;
      if(i==0)
	R[i] = R[i]*0.1;
      ii = ii +1.0;
    }
  
  for(beta = beta_ini; beta <= beta_end; beta = beta + Delta_beta)
    { 
      for(gamma = gamma_ini; gamma <= gamma_end;  gamma = gamma + Delta_gamma)
	{     
	  for(a = a_ini; a <= a_end;  a = a + Delta_a)
	    {
	        // PARA EVITAR INDETERMINACIONES EN ALGUNOS DENOMINADORES
		 // if (a != a)
		  //  {
		        for(M = M_ini; M < M_end;  M = M + Delta_M)
			    {
			      // ESTE CICLO ES PARA R (RADIO PROYECTADO) YA QUE SIGMA DEPENDE DE R, ADEMAS VA A SER EL LIMITE INFERIOR DE LA INTEGRAL
			      			      
			      for(i = 0; i < K; i ++)          
				{
				  evaluate_integral(R[i], beta, gamma, a, M, &sig_p[i], &R_arcmin[i]);
				  
				}

			      gsl_interp_accel *acc  =   gsl_interp_accel_alloc ();
			      gsl_spline *spline     =   gsl_spline_alloc (gsl_interp_cspline, 90);
			      gsl_spline_init (spline, R_arcmin, sig_p, 90);
			      
			      chi2 = 0.0;
			      for(j=0; j<NUMBER_OF_SIGMA_BINS; j++)
				{
				  sig_p_i = gsl_spline_eval (spline, R_o[j], acc);
				  chi2 += ( (sig_p_i-sig_p_o[j])*(sig_p_i-sig_p_o[j]) ) / numb_error[j];
				}

			      gsl_spline_free (spline);
			      gsl_interp_accel_free (acc);
			      
			      
			      if(chi2 < chi_min)
				{
				  chi_min   = chi2;
				  a_min     = a;
				  M_min     = M;
				  gamma_min = gamma;
				  beta_min  = beta;
				}
			      
			      if((counter%1000) == 0)
				printf("Ready %d iterations of %d\n",counter, Ntotal_iterations);
			      counter++;
			      
			    }// for M	      
		   // }// if for the singularity
	    }// for a
	}//for gamma
    }//for beta
  
  //printf ("\n");
  
  chi_cua=fopen("chi_cuadrado.dat","w"); 
  
  for(i = 0; i < K; i ++)          
    {
      
      evaluate_integral(R[i], beta_min, gamma_min, a_min, M_min, &sig_p[i], &R_arcmin[i]);
      fprintf(chi_cua,"%16.8e %16.8e\n", R_arcmin[i], sig_p[i]);
      
    }
  
  fclose(chi_cua);
  
  // MUESTRO CUALES SON LOS PARAMETROS QUE MEJOR SE AJUSTAN A LOS DATOS OBSERVACIONALES
  
  printf("xi=%16.8lf beta=%16.8lf a/pc=%16.8lf M/Msun=%16.8elf gamma=%lf\n",
	 chi_min, beta_min, 1000*a_min, (1.0e10)*M_min, gamma_min);
  
  
  script = fopen( "script.gpl", "w" );
  fprintf( script, "plot 'sigma.dat'pt 7 ps 1.2\n" );
  fprintf( script, "replot 'chi_cuadrado.dat' u 1:2 w l\n");
  fprintf( script, "reset\n");
  fprintf(script, "set grid\nset terminal png\nset output 'sigma.png'\nset nokey\n");
  fprintf( script, "set title 'Sigma Proyectada vs R'\n" );
  fprintf( script, "set xrange [0:40]\n" );
  fprintf( script, "set yrange [4:14]\n" );
  fprintf( script, "set xlabel 'Radius in Arcmin'\n" );
  fprintf( script, "set ylabel 'Projected velocity dispersion in km/s'\n" );
  fprintf( script, "plot 'sigma.dat'pt 7 ps 1.2\n" );
  fprintf( script, "replot 'chi_cuadrado.dat' u 1:2 w l\n");
  fprintf( script, "pause -1\n");
  fclose(script);
  
  warn = system("gnuplot script.gpl");
  
  return(warn);
}
