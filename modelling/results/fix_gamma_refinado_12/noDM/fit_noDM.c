#include <stdio.h>
#include <math.h>
#include <gsl/gsl_integration.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_spline.h>

//////////////////////////////////////////////////////////////////////////////
// HERNQUIST MODEL FOR THE PROJECTED VELOCITY DISPERSION IN OMEGA CENTAURI  //
//////////////////////////////////////////////////////////////////////////////

/*  SISTEM OF UNITS 
    
    1 unit of mass   = 1.0e5Msun
    1 unit of lenght = 1.0 pc
    1 unit velocity  = 1km/s
    G = 430.071
    
    Hubble (internal units) = 0.0001
    UnitMass_in_g = 1.989e+38 
    UnitTime_in_s = 3.08568e+13 
    UnitVelocity_in_cm_per_s = 100000 
    UnitDensity_in_cgs = 6.76991e-18 
    UnitEnergy_in_cgs = 1.989e+48 
    
*/

#define K 90                 //90 PARSECS, OUT OF THE ~60 PC OF OMEGA CENTAURI
#define G 430.071            
#define EPSILON 1            // TO SOFTEN INDETERMINATIONS
#define DISTANCE 4808.39
#define LIMIT_RADIUS 1500
#define ZERO 1.9e-10

struct param
{
  double beta, a, M, R;  
};

// THE FILE ROUTINES.C HAS THE LARGE EQUATIONS OF THE INTEGRAND 

#include "routines_noDM.c"

//THIS IS THE FUNCTION THAT CONTAINS ALL THE CALCULATIONS, INCLUDING THE INTEGRALS AND INTERPOLATIONS 

int evaluate_integral(double R, double beta, double gamma, double a, double M, double *sig_p, double *R_arcmin)
{
  
  int Nint = 10000;    // NUMBER OF INTERVALS
  double result, error, s, aux, X_s, I_R;
  
  gsl_integration_workspace *z = gsl_integration_workspace_alloc(Nint);
  gsl_function F1;
  
  struct param params;
  
  params.beta  =  beta;
  params.a     =  a;
  params.M     =  M;       
  params.R     =  R;
  
  //////////////////////////////////////////////////////////////
  //  THIS IS JUST TO PRINT THE INTEGRAND TO SEE ITS BEHAVIOR  //
  ///////////////////////////////////////////////////////////////
  /* 
     {
     FILE *pf=NULL;
     double r;
     pf=fopen("puto_archivo.dat","w");
     for(r=0; r<LIMIT_RADIUS; r=r+1)
     {
     fprintf(pf,"%16.8e %16.8e\n",r, integrando(r, &params));
     }
     fclose(pf);
     exit(0);
     } 
  */
  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////
  
  F1.function = &integrando;
  F1.params = &params;
  
  //gsl_integration_qags(&F1, R, LIMIT_RADIUS, 1.0e-8, 1e-6, Nint, z, &result, &error); 
  gsl_integration_qag(&F1, R, LIMIT_RADIUS, 1.0e-5, 1e-5, Nint, 1, z, &result, &error); 
  //printf("R = %lf I=%lf\n",R, result);
  
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
  
  *sig_p = sqrt( (G*result) / (gamma*I_R*M_PI) ) ;
  
  //THIS LINE CONVERTS FROM PARSECS TO ARCMIN SINCE THE OBSERVATIONAL DATA IS IN THOSE UNITS
  *R_arcmin = R*3437.75/DISTANCE;
  
  gsl_integration_workspace_free(z);
  
  return 0;  
}

// THE MAIN PROGRAM WILL RUN FOR ALL THE COMBINATIONS OF PARAMETERS AND FIT THE XI SQUARE TO GET THE BEST COMBINATION

int main(int argc, char *argv[])
{
  
  int warn, i, j, nread, NUMBER_OF_SIGMA_BINS; 
  double X_s, I_R, s, R[90], R_arcmin[90], sig_p[90], a, M, beta;
  double R_o[12], sig_p_o[12], numb_error[12], chi2, a_min, M_min, beta_min, sig_p_i;
  double chi_min = 100000000000000000.0;
  double gamma, gamma_min, aux;
  char *infile;
  FILE *script,*sigma,*best_model,*results;
  
  double gamma_ini, gamma_end, Delta_gamma;
  double a_ini, a_end, Delta_a;
  double M_ini, M_end, Delta_M;
  double beta_ini, beta_end, Delta_beta;
  
  infile = argv[1];                          // INPUT FIL WITH VELOCITY DISPERSION BINS
  NUMBER_OF_SIGMA_BINS = atoi(argv[2]);      // EVENTUALLY 12!
  
  if(argc != 3)
    {
      printf(" Execute as ./exec <infile> <number of lines>\n");    // THIS WILL TELL HOW TO PROPERLY DO THE EXECUTION
      exit(0);
    }
  
  // READ THE FILE WITH THE OBSERVATIONAL DATA
  sigma=fopen(infile,"r");                      
  for(j=0.0; j<NUMBER_OF_SIGMA_BINS; j++)
    nread = fscanf(sigma,"%lf %lf %lf", &R_o[j], &sig_p_o[j], &numb_error[j]);
  fclose(sigma);
  
  // THE VARIATIONS OF THE PARAMETERS

  gamma = 2.5;
  
  a_ini      = 3.0;        a_end      = 3.4;      Delta_a      = 0.03;
  M_ini      = 13.4;       M_end      = 17.4;      Delta_M      = 0.03;
  beta_ini   = 0.001;        beta_end   = 0.3;     Delta_beta   = 0.01;
  
  // THIS LINE CALCULATES THE NUMBER OF ITERATIONS FOR THE USER'S REFERENCE
  int Ntotal_iterations, counter, cuenta;
  Ntotal_iterations = (int) ( ((beta_end-beta_ini)/Delta_beta)*((a_end-a_ini)/Delta_a)*((M_end-M_ini)/Delta_M) );
  
  printf("  Running %d iterations in parameter space\n", Ntotal_iterations);
  
  counter=0;
  
  //DEFINITION OF THE VECTOR R[i]
  
  for(i=0;i<K;i++)
    {
      R[i] = (i+1.0);
      if(i==0)
	R[i] = R[i]*0.1;
    }
  
  for(beta = beta_ini; beta <= beta_end; beta = beta + Delta_beta)
    { 
      for(a = a_ini; a <= a_end;  a = a + Delta_a)
	{
	  for(M = M_ini; M < M_end;  M = M + Delta_M)
	    {
	      for(i = 0; i < K; i ++)
		{
		  evaluate_integral(R[i], beta, gamma, a, M, &sig_p[i], &R_arcmin[i]);
		  //printf("%d R = %lf I=%lf\n",i, log10(R[i]), sig_p[i]);
		}
	      
	      // ALLOC MEMORY FOR THE INTERPOLATION IN EACH STEP
	      gsl_interp_accel *acc  =  gsl_interp_accel_alloc ();
	      gsl_spline *spline     =  gsl_spline_alloc (gsl_interp_cspline, 90);
	      
	      gsl_spline_init (spline, R_arcmin, sig_p, 90);
	      
	      chi2 = 0.0;
	      for(j=0; j<NUMBER_OF_SIGMA_BINS; j++)
		{
		  sig_p_i = gsl_spline_eval (spline, R_o[j], acc);
		  chi2 += ( (sig_p_i-sig_p_o[j])*(sig_p_i-sig_p_o[j]) ) / numb_error[j];
		  //printf("%16.8lf\t %16.8lf\n", sig_p_i, chi2);
		}
	      
	      // FREE MEMORY
	      gsl_spline_free (spline);
	      gsl_interp_accel_free (acc);
	      
	      // CALCULATION OF XI SQUARE TO FIND THE OPTIMIZED PARAMETERS
	      if(chi2 < chi_min)
		{
		  chi_min   = chi2;
		  a_min   = a;
		  M_min   = M;
		  gamma_min = gamma;
		  beta_min = beta;
		}
	      
	      if((counter%1000) == 0)
		printf("Ready %d iterations of %d\n",counter, Ntotal_iterations);
	      counter++;
	      
	    }// for M
	}// for a
    }//for beta
  
  best_model=fopen("best_model.dat","w"); 
  
  for(i = 0; i < K; i ++)          
    {      
      evaluate_integral(R[i], beta_min, gamma_min, a_min, M_min, &sig_p[i], &R_arcmin[i]);
      fprintf(best_model,"%16.8e %16.8e\n", R_arcmin[i], sig_p[i]); 
    }
  
  fclose(best_model);
  
  // THIS ARE THE OPTIMIZED PARAMETERS
  
  printf("xi = %16.8lf\t beta = %16.8lf\t a/pc = %16.8lf\t M/Msun = %16.8lf\t gamma = %lf\n",
	 chi_min, beta_min, a_min, (1.0e5)*M_min, gamma_min);
  
  results = fopen("results.dat", "w");
  fprintf(results,"xi= %16.8lf\t beta= %16.8lf\t a/pc= %16.8lf\t M/Msun= %16.8lf\t gamma= %lf\n",
	  chi_min, beta_min, a_min, (1.0e5)*M_min, gamma_min);
  
  fclose(results);
  
  script = fopen( "script.gpl", "w" );
  fprintf( script, "plot 'sigma.dat'pt 7 ps 1.2\n" );
  //fprintf( script, "replot 'best_model.dat' u 1:2 w l\n");
  //fprintf( script, "reset\n");
  fprintf(script, "set grid\nset terminal png\nset output 'sigma.png'\nset nokey\n");
  fprintf( script, "set title 'Sigma Proyectada vs R'\n" );
  fprintf( script, "set xrange [0:40]\n" );
  fprintf( script, "set yrange [4:14]\n" );
  fprintf( script, "set xlabel 'Radius in Arcmin'\n" );
  fprintf( script, "set ylabel 'Projected velocity dispersion in km/s'\n" );
  //fprintf( script, "plot 'sigma.dat'pt 7 ps 1.2\n" );
  fprintf( script, "replot 'best_model.dat' u 1:2 w l\n");
  //fprintf( script, "pause -1\n");
  fclose(script);
  
  warn = system("gnuplot script.gpl");
  
  return(warn);
}

