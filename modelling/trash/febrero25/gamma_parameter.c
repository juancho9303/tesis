#include <stdio.h>
#include <math.h>
#include <gsl/gsl_integration.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_spline.h>

/*
  SE UTILIZA UN MODELO DE HERNQUIST MODIFICADO PARA ENCONTRAR LA
  FORMA DE LA DISPERSION DE VELOCIDADES PROYECTADA Y PODER AJUSTAR
  LOS PARAMETROS QUE MEJOR SE AJUSTEN A LAS OBSERVACIONES   
*/

#define K 90           //90 parsecs que esta por encima de los ~60pc de omega centauri
//#define GAMMA 1.0        //razon masa luminosidad
#define G 43007.1        //unidades de Gadget
#define EPSILON 1     //para suavizar denominadores para evitar indeterminaciones

/* UNIDADES:

   UnitMass_in_g = 1.989e+43 
   UnitTime_in_s = 3.08568e+16 
   UnitVelocity_in_cm_per_s = 100000 
   Unidad de Masa = 10^10 Msun/h
   Unidad de longitud = 1kpc/h
*/

// GENERO LA ESTRUCTURA DE PARAMETROS QUE VOY A AJUSTAR
struct param
{
  double beta, a_s, a_dm, M_s, M_dm;  
};

double R;

// ACA SE DEFINE LA INTEGRAL QUE CONTIENE TODOS LOS TERMINOS DE LAS CONTRIBUCIONES DE MASA ESTELAR Y MATERIA OSCURA Y QUE DEPENDE DE r 

double integrando(double r, void * params) 
{
  struct param parameters = *(struct param *) params;
  
  double a_s    =  parameters.a_s;
  double a_dm   =  parameters.a_dm;
  double M_s    =  parameters.M_s;
  double M_dm   =  parameters.M_dm;
  double beta   =  parameters.beta;
  double alpha, b1, b2, b3, b4, b5, c1, c2, c3, c4, c5, M_sM_dm, M_dmM_s, M_sM_s, M_dmM_dm;
  
  alpha = ((1.0 - beta*((R*R)/(r*r)))*((r)/(sqrt(r*r-R*R))));
  
  M_sM_s = M_s*M_s*a_s*(((12.0*pow((a_s+r),4.0)*log((a_s+r)/(r))) - a_s*(25.0*a_s*a_s*a_s+52.0*a_s*a_s*r+42.0*a_s*r*r+12.0*r*r*r))/(12.0*pow(a_s,5)*pow((a_s+r),4)));
  
  M_dmM_dm = M_dm*M_dm*a_dm*(((12.0*pow((a_dm+r),4.0)*log((a_dm+r)/(r))) - a_dm*(25.0*a_dm*a_dm*a_dm+52.0*a_dm*a_dm*r+42.0*a_dm*r*r+12.0*r*r*r))/(12.0*pow(a_dm,5)*pow((a_dm+r),4)));
  
  b1 = 2.0*pow(a_s,3.0)*pow((a_s-a_dm),4.0)*a_dm*a_dm*(a_s+r)*(a_s+r)*(a_dm+r);
  b2 = 2.0*pow((a_s-a_dm),4.0)*(a_s+r)*(a_s+r)*(a_dm+r)*log(r);
  b3 = 2.0*a_dm*a_dm*(6.0*a_s*a_s-4.0*a_s*a_dm+a_dm*a_dm)*(a_s+r)*(a_s+r)*(a_dm+r)*log(a_s+r);
  b4 = 2.0*pow(a_s,4.0)+4.0*pow(a_s,3.0)*r-2.0*a_dm*a_dm*r*(a_dm+r)+3.0*a_s*a_dm*(-a_dm*a_dm+a_dm*r+2.0*r*r)+a_s*a_s*(7.0*a_dm*a_dm+7.0*a_dm*r+2.0*r*r);
  b5 = 2.0*a_s*a_s*(a_s-4.0*a_dm)*(a_s+r)*(a_s+r)*(a_dm+r)*log(a_dm+r);
  
  M_sM_dm = M_s*M_dm*((-1.0)/(b1))*(b2-b3+a_s*((a_s-a_dm)*a_dm*b4-b5));
  
  c1 = 2.0*pow(a_dm,3)*pow((a_dm-a_s),4.0)*a_s*a_s*(a_dm+r)*(a_dm+r)*(a_s+r);
  c2 = 2.0*pow((a_dm-a_s),4.0)*(a_dm+r)*(a_dm+r)*(a_s+r)*log(r);
  c3 = 2.0*a_s*a_s*(6.0*a_dm*a_dm-4.0*a_dm*a_s+a_s*a_s)*(a_dm+r)*(a_dm+r)*(a_s+r)*log(a_dm+r);
  c4 = 2.0*pow(a_dm,4.0)+4.0*pow(a_dm,3.0)*r-2.0*a_s*a_s*r*(a_s+r)+3.0*a_dm*a_s*(-a_s*a_s+a_s*r+2.0*r*r)+a_dm*a_dm*(7.0*a_s*a_s+7.0*a_s*r+2.0*r*r);
  c5 = 2.0*a_dm*a_dm*(a_dm-4.0*a_s)*(a_dm+r)*(a_dm+r)*(a_s+r)*log(a_s+r);
  
  M_dmM_s = M_dm*M_s*((-1.0)/(c1))*(c2-c3+a_dm*((a_dm-a_s)*a_s*c4-c5));
  
  double integrando = alpha*(M_sM_s + M_dmM_dm + M_sM_dm + M_dmM_s);
  
  return integrando;
}

double integrando2(double r, void * params) 
{
  struct param parameters = *(struct param *) params;
  
  double a_s_min    =  parameters.a_s;
  double a_dm_min   =  parameters.a_dm;
  double M_s_min    =  parameters.M_s;
  double M_dm_min   =  parameters.M_dm;
  double beta   =  parameters.beta;
  double alpha, b1, b2, b3, b4, b5, c1, c2, c3, c4, c5, M_sM_dm, M_dmM_s, M_sM_s, M_dmM_dm;
  
  alpha = ((1.0 - beta*((R*R)/(r*r)))*((r)/(sqrt(r*r-R*R))));
  
  M_sM_s = M_s_min*M_s_min*a_s_min*(((12.0*pow((a_s_min+r),4.0)*log((a_s_min+r)/(r))) - a_s_min*(25.0*a_s_min*a_s_min*a_s_min+52.0*a_s_min*a_s_min*r+42.0*a_s_min*r*r+12.0*r*r*r))/(12.0*pow(a_s_min,5)*pow((a_s_min+r),4)));
  
  M_dmM_dm = M_dm_min*M_dm_min*a_dm_min*(((12.0*pow((a_dm_min+r),4.0)*log((a_dm_min+r)/(r))) - a_dm_min*(25.0*a_dm_min*a_dm_min*a_dm_min+52.0*a_dm_min*a_dm_min*r+42.0*a_dm_min*r*r+12.0*r*r*r))/(12.0*pow(a_dm_min,5)*pow((a_dm_min+r),4)));
  
  b1 = 2.0*pow(a_s_min,3.0)*pow((a_s_min-a_dm_min),4.0)*a_dm_min*a_dm_min*(a_s_min+r)*(a_s_min+r)*(a_dm_min+r);
  b2 = 2.0*pow((a_s_min-a_dm_min),4.0)*(a_s_min+r)*(a_s_min+r)*(a_dm_min+r)*log(r);
  b3 = 2.0*a_dm_min*a_dm_min*(6.0*a_s_min*a_s_min-4.0*a_s_min*a_dm_min+a_dm_min*a_dm_min)*(a_s_min+r)*(a_s_min+r)*(a_dm_min+r)*log(a_s_min+r);
  b4 = 2.0*pow(a_s_min,4.0)+4.0*pow(a_s_min,3.0)*r-2.0*a_dm_min*a_dm_min*r*(a_dm_min+r)+3.0*a_s_min*a_dm_min*(-a_dm_min*a_dm_min+a_dm_min*r+2.0*r*r)+a_s_min*a_s_min*(7.0*a_dm_min*a_dm_min+7.0*a_dm_min*r+2.0*r*r);
  b5 = 2.0*a_s_min*a_s_min*(a_s_min-4.0*a_dm_min)*(a_s_min+r)*(a_s_min+r)*(a_dm_min+r)*log(a_dm_min+r);
  
  M_sM_dm = M_s_min*M_dm_min*((-1.0)/(b1))*(b2-b3+a_s_min*((a_s_min-a_dm_min)*a_dm_min*b4-b5));
  
  c1 = 2.0*pow(a_dm_min,3)*pow((a_dm_min-a_s_min),4.0)*a_s_min*a_s_min*(a_dm_min+r)*(a_dm_min+r)*(a_s_min+r);
  c2 = 2.0*pow((a_dm_min-a_s_min),4.0)*(a_dm_min+r)*(a_dm_min+r)*(a_s_min+r)*log(r);
  c3 = 2.0*a_s_min*a_s_min*(6.0*a_dm_min*a_dm_min-4.0*a_dm_min*a_s_min+a_s_min*a_s_min)*(a_dm_min+r)*(a_dm_min+r)*(a_s_min+r)*log(a_dm_min+r);
  c4 = 2.0*pow(a_dm_min,4.0)+4.0*pow(a_dm_min,3.0)*r-2.0*a_s_min*a_s_min*r*(a_s_min+r)+3.0*a_dm_min*a_s_min*(-a_s_min*a_s_min+a_s_min*r+2.0*r*r)+a_dm_min*a_dm_min*(7.0*a_s_min*a_s_min+7.0*a_s_min*r+2.0*r*r);
  c5 = 2.0*a_dm_min*a_dm_min*(a_dm_min-4.0*a_s_min)*(a_dm_min+r)*(a_dm_min+r)*(a_s_min+r)*log(a_s_min+r);
  
  M_dmM_s = M_dm_min*M_s_min*((-1.0)/(c1))*(c2-c3+a_dm_min*((a_dm_min-a_s_min)*a_s_min*c4-c5));
  
  double integrando2 = alpha*(M_sM_s + M_dmM_dm + M_sM_dm + M_dmM_s);
  
  return integrando2;
}

// AHORA INICIO EL PROGRAMA PRINCIPAL QUE VA A HACER TODOS LOS CALCULOS PARA ENCONTRAR LA DSPERSION DE VELOCIDADES PROYECTADA

int main()
{
  
  // DEFINO LOS PUNTEROS DE LOS ARCHIVOS, LAS VARIABLES DE TIPO ENTERO Y DOUBLE Y LA ALOCACION DE MEMORIA DE LA RUTINA DE GSL QUE VOY A USAR PARA LA INTEGRAL
  
  FILE *mod,*script,*sigma,*chi_cua;
  int warn, t, i, j, k;
  int Nint = 1000; // Numero de intervalos
  double X_s, I_R, s, R[90], R_arcmin[90], sig_p[90], rho, re1, re2, result1, error1, a_dm, a_s, M_s, M_dm, beta, R_o[12], sig_p_o[12], chi2, chi_t, otra, a_dm_min, a_s_min, M_dm_min, M_s_min, R_arcmin_i, sig_p_i;
  double chi_min = 1000000.0;
  double gamma, gamma_min;
  struct param params;
  gsl_integration_workspace *z = gsl_integration_workspace_alloc(Nint);
  gsl_function F1;
  gsl_function F2;
  gsl_interp_accel *acc  =   gsl_interp_accel_alloc ();
  gsl_spline *spline     =   gsl_spline_alloc (gsl_interp_cspline, 90);
  
  sigma=fopen("sigma.dat","r"); 
  
  // ESTE CICLO ES PARA LEER EL ARCHIVO DE DATOS CON LOS DATOS OBSERVACIONALES DE SIGMA
  
  for(j=0.0; j<12; j++)
    {        
      fscanf(sigma, "%lf %lf", &R_o[j], &sig_p_o[j]);
    }      
  
  fclose(sigma);
  
  // HAGO LAS VARIACIONES DE LOS PARAMETROS
  
  beta  =  0.0001;
  
  mod=fopen("model.dat","w"); 
  chi_cua=fopen("chi_cuadrado.dat","w"); 
  
  
  for(gamma = 0.1; gamma <= 3.0;  gamma = gamma + 0.5)
    { 
      for(a_dm = 0.005; a_dm <= 0.06;  a_dm = a_dm + 0.01)
	{
	  for(a_s = 0.005; a_s <= 0.06;  a_s = a_s + 0.01)
	    {
	      // PARA EVITAR INDETERMINACIONES EN ALGUNOS DENOMINADORES
	      if (a_s != a_dm)
		{
		  for(M_dm = 0.00001; M_dm < 0.0008;  M_dm = M_dm + 0.0001)
		    {
		      for(M_s = 0.00001; M_s < 0.0008;  M_s = M_s + 0.0001)
			{
			  params.beta  =  beta;
			  params.a_s   =  a_s;
			  params.a_dm  =  a_dm;
			  params.M_s   =  M_s;
			  params.M_dm  =  M_dm;       
			  
			  // ESTE CICLO ES PARA R (RADIO PROYECTO) YA QUE SIGMA DEPENDE DE R, ADEMAS VA A SER EL LIMITE INFERIOR DE LA INTEGRAL
			  
			  for(i = 0; i < K; i ++)          
			    {
			      R[i] = (i+1.0)/(1000.0);
			      F1.function = &integrando;
			      F1.params = &params;
			      
			      gsl_integration_qags(&F1, R[i], 0.1, 0, 1e-4, Nint, z, &result1, &error1); 
			      
			      re1 = result1;
			      s = R[i]/a_s;
			      
			      // ES NECESARIO PONER ESTOS CONDICIONALES YA QUE LA FUNCION X(S) ESTA DEFINIDA A TRAMOS
			      
			      if (s < 1.0 - 1.0e-9)
				{      	
				  X_s = (1.0 / sqrt(1.0 - s*s)) * log((1.0+(sqrt(1.0-s*s)))/(s));
				  I_R = (M_s/(2.0*M_PI*a_s*a_s*gamma*(1.0-s*s)*(1.0-s*s)))*((2.0+s*s)*X_s-3.0);
				}
			      
			      if(s >= 1.0-1.0e-10 && s <= 1.0+1.0e-10)
				if(s = 1.0)
				  {
				    X_s = 1.0;
				I_R = (2.0*M_s)/(15.0*M_PI*a_s*a_s*gamma);
				  }
			      
			      if (s > 1.0 + 1.0e-9)
				{	       	  
				  X_s = (1.0 / sqrt(s*s - 1.0)) * acos(1.0/s);
				  I_R = (M_s/(2.0*M_PI*a_s*a_s*gamma*(1.0-s*s)*(1.0-s*s)))*((2.0+s*s)*X_s-3.0);
				}      
			      
			      // SE HACEN LOS CALCULOS UNA VEZ SE TIENE EL VALOR DE LA INTEGRAL
			      
			      sig_p[i] = sqrt(((G)/(gamma*(I_R)*M_PI))*re1);
			      //CONVIERTO DE KILOPARSECS A MINUTOS DE ARCO
			      R_arcmin[i] = R[i]/(4.80839)*3437.75;
			      //printf ("%g %g\n", R_arcmin[i], sig_p[i]);
			    }
			  
			  gsl_spline_init (spline, R_arcmin, sig_p, 90);
			  
			  for(R_arcmin_i = R_arcmin[0]; R_arcmin_i < R_arcmin[89]; R_arcmin_i += 0.001)
			    {
			      sig_p_i = gsl_spline_eval (spline, R_arcmin_i, acc);
			      //printf ("%g %g\n", R_arcmin_i, sig_p_i);
			      for(j=0.0; j<12.0; j=j+1.0)
				{
				  if( R_arcmin_i - R_o[j] < 0.001 && R_arcmin_i - R_o[j] > -0.001 )
				    //if( R_arcmin_i = R_o[j] )
				    {
				      if(sig_p_i > 0.0)
					{
					  chi2 = (sig_p_i-sig_p_o[j])*(sig_p_i-sig_p_o[j]);
					  chi_t = chi_t + chi2;
					}       
				    }
				}
			    }
			  
			  //printf("%g\n", chi_t);
			  
			  if(chi_t < chi_min)
			    {
			      chi_min   = chi_t;
			      a_dm_min  = a_dm;
			      a_s_min   = a_s;
			      M_dm_min  = M_dm;
			      M_s_min   = M_s;
			      gamma_min = gamma;
			      //fprintf(mod,"%16.8e\t %16.8e\n", R_min, sig_p);
			      
			      params.beta  =  beta;
			      params.a_s   =  a_s_min;
			      params.a_dm  =  a_dm_min;
			      params.M_s   =  M_s_min;
			      params.M_dm  =  M_dm_min;	  
			    }
			  //printf("%lf\n", chi_t);
			  chi_t = 0.0;
			  
			}
		    }
		}
	    }
	}
    }
  
  params.a_s   =  a_s_min;
  params.a_dm  =  a_dm_min;
  params.M_s   =  M_s_min;
  params.M_dm  =  M_dm_min;
  
  for(i = 0; i < K; i ++)          
    { 
      R[i] = (i+1.0)/(1000.0);
      F2.function = &integrando2;
      F2.params = &params;
      gsl_integration_qags(&F2, R[i], 0.1, 0, 1e-4, Nint, z, &result1, &error1); 
      re1 = result1;
      s = R[i]/a_s_min;
      
      if (s < 1.0 - 1.0e-9)
	{      	
	  X_s = (1.0 / sqrt(1.0 - s*s)) * log((1.0+(sqrt(1.0-s*s)))/(s));
	  I_R = (M_s_min/(2.0*M_PI*a_s_min*a_s_min*gamma_min*(1.0-s*s)*(1.0-s*s)))*((2.0+s*s)*X_s-3.0);
	}
      
      if(s >= 1.0-1.0e-10 && s <= 1.0+1.0e-10)
	if(s = 1.0)
	  {
	    X_s = 1.0;
	    I_R = (2.0*M_s_min)/(15.0*M_PI*a_s_min*a_s_min*gamma_min);
	  }
      
      if (s > 1.0 + 1.0e-9)
	{	       	  
	  X_s = (1.0 / sqrt(s*s - 1.0)) * acos(1.0/s);
	  I_R = (M_s_min/(2.0*M_PI*a_s_min*a_s_min*gamma_min*(1.0-s*s)*(1.0-s*s)))*((2.0+s*s)*X_s-3.0);
	}      
      
      sig_p_i = sqrt(((G)/(gamma_min*(I_R)*M_PI))*re1);
      R_arcmin_i = R[i]/(4.80839)*3437.75;
      fprintf(chi_cua,"%16.8e\t %16.8e\n", R_arcmin_i, sig_p_i);
    }
  
  // MUESTRO CUALES SON LOS PARAMETROS QUE MEJOR SE AJUSTAN A LOS DATOS OBSERVACIONALES
  
  printf("xi=%lf\t a_dm=%lf\t a_s=%lf\t M_dm=%lf\t M_s=%lf gamma=%lf\n",chi_min,a_dm_min,a_s_min,M_dm_min,M_s_min,gamma_min);
  
  fclose(mod);
  fclose(chi_cua);
  gsl_integration_workspace_free(z);
  gsl_spline_free (spline);
  gsl_interp_accel_free (acc);
  
  script = fopen( "script.gpl", "w" );
  fprintf( script, "plot 'sigma.dat'pt 7 ps 1.2\n" );
  fprintf(script, "set grid\nset terminal png\nset output 'sigma.png'\nset nokey\n");
  fprintf( script, "set title 'Sigma Proyectada vs R'\n" );
  fprintf( script, "set xrange [0:40]\n" );
  fprintf( script, "set yrange [4:14]\n" );
  fprintf( script, "set xlabel 'Radius in Arcmin'\n" );
  fprintf( script, "set ylabel 'Projected velocity dispersion in km/s'\n" );
  fprintf( script, "replot 'chi_cuadrado.dat' u 1:2 w l\n");
  fclose(script);
  
  warn = system("gnuplot script.gpl");
  
  return(warn);
}
