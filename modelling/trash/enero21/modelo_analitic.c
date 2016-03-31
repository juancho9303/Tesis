#include <stdio.h>
#include <math.h>
#include <gsl/gsl_integration.h>

// SE UTILIZA EL MODELO DE HERNQUIST PARA ENCONTRAR LA FORMA DE LA DISPERSION DE VELOCIDADES PROYECTADA Y PODER AJUSTAR LOS PARAMETROS QUE MEJOR SE AJUSTEN A LAS OBSERVACIONES

#define N 10
#define M 10e-3
#define GAMMA 1.2
#define G 43007.1

struct param{
  double beta, a_s, a_dm, M_dm, M_s;
};

double R;

// ACA SE DEFINE LA INTEGRAL QUE CONTIENE TODOS LOS TERMINOS DE LAS CONTRIBUCIONES DE MASA ESTELAR Y MATERIA OSCURA 

double tot (double r, void * params) 
{
  struct param parameters = *(struct param *) params;
  
  double a_s = parameters.a_s;
  double a_dm = parameters.a_dm;
  double M_s = parameters.M_s;
  double M_dm = parameters.M_dm;
  double beta = parameters.beta;
  double alpha, a1, a2, a3, a4, a5, b1, b2, b3, b4, b5;
  double M_sM_dm, M_dmM_s, M_sM_s, M_dmM_dm;
  
  alpha = ((1.0 - beta*((R*R)/(r*r)))*((r)/(sqrt(r*r-R*R))));
  
  M_sM_s = M_s*M_s*a_s*(((12.0*pow((a_s+r),4.0)*log((a_s+r)/(r))) - a_s*(25.0*a_s*a_s*a_s+52.0*a_s*a_s*r+42.0*a_s*r*r+12.0*r*r*r))/(12.0*pow(a_s,5)*pow((a_s+r),4)));
  
  M_dmM_dm = M_dm*M_dm*a_dm*(((12.0*pow((a_dm+r),4.0)*log((a_dm+r)/(r))) - a_dm*(25.0*a_dm*a_dm*a_dm+52.0*a_dm*a_dm*r+42.0*a_dm*r*r+12.0*r*r*r))/(12.0*pow(a_dm,5)*pow((a_dm+r),4)));
  
  //----------------------------------------------------------------
  // Paso a paso sin hacer reducciones para los terminos cruzados.
  //--------------------------------------------------------------
  
  a1 = 2.0*pow(a_s,3.0)*pow((a_s-a_dm),4.0)*a_dm*a_dm*(a_s+r)*(a_s+r)*(a_dm+r);
  a2 = 2.0*pow((a_s-a_dm),4.0)*(a_s+r)*(a_s+r)*(a_dm+r)*log(r);
  a3 = 2.0*a_dm*a_dm*(6.0*a_s*a_s-4.0*a_s*a_dm+a_dm*a_dm)*(a_s+r)*(a_s+r)*(a_dm+r)*log(a_s+r);
  a4 = 2.0*pow(a_s,4.0)+4.0*pow(a_s,3.0)*r-2.0*a_dm*a_dm*r*(a_dm+r)+3.0*a_s*a_dm*(-a_dm*a_dm+a_dm*r+2.0*r*r)+a_s*a_s*(7.0*a_dm*a_dm+7.0*a_dm*r+2.0*r*r);
  a5 = 2.0*a_s*a_s*(a_s-4.0*a_dm)*(a_s+r)*(a_s+r)*(a_dm+r)*log(a_dm+r);
  
  M_sM_dm = M_s*M_dm*((-1.0)/(a1))*(a2-a3+a_s*((a_s-a_dm)*a_dm*a4-a5));
  //printf ("M_sM_dm  = %6lf\t % .18f\n", r, M_sM_dm);
  
  b1 = 2.0*pow(a_dm,3)*pow((a_dm-a_s),4.0)*a_s*a_s*(a_dm+r)*(a_dm+r)*(a_s+r);
  b2 = 2.0*pow((a_dm-a_s),4.0)*(a_dm+r)*(a_dm+r)*(a_s+r)*log(r);
  b3 = 2.0*a_s*a_s*(6.0*a_dm*a_dm-4.0*a_dm*a_s+a_s*a_s)*(a_dm+r)*(a_dm+r)*(a_s+r)*log(a_dm+r);
  b4 = 2.0*pow(a_dm,4.0)+4.0*pow(a_dm,3.0)*r-2.0*a_s*a_s*r*(a_s+r)+3.0*a_dm*a_s*(-a_s*a_s+a_s*r+2.0*r*r)+a_dm*a_dm*(7.0*a_s*a_s+7.0*a_s*r+2.0*r*r);
  b5 = 2.0*a_dm*a_dm*(a_dm-4.0*a_s)*(a_dm+r)*(a_dm+r)*(a_s+r)*log(a_s+r);
  
  M_dmM_s = M_dm*M_s*((-1.0)/(b1))*(b2-b3+a_dm*((a_dm-a_s)*a_s*b4-b5));
  //printf ("M_dmM_s  = %6lf\t % .18f\n", r, M_dmM_s);
  
  double tot = alpha*(M_sM_s + M_dmM_dm + M_sM_dm + M_dmM_s);
  //double tot = alpha*(M_sM_s + M_dmM_dm);
  //double tot = alpha*(M_sM_dm + M_dmM_s);
  
  return tot;
}

int main(){
  
  FILE *mod,*script;
  int warn;
  int Nint = 10000; // Numero de intervalos
  double X_s, I_R, s, R, sig_p, rho, re1, re2, result1, error1, result2, error2, a_dm, a_s, M_s, M_dm, beta;
  struct param params; // structura de parametros
  gsl_integration_workspace *z = gsl_integration_workspace_alloc(Nint);
  gsl_function F1;
  
  // PARAMETROS
  

  a_dm = 0.9;
  a_s = 1.0;
  M_s = 2.0;
  M_dm = 2.0;
  beta = 2.0;
  
  params.beta = beta;
  params.a_s = a_s;
  params.a_dm = a_dm;
  params.M_s = M_s;
  params.M_dm = M_dm;
  
  mod=fopen("model.dat","w");
  
  for(R = 0.01; R < N; R=R+0.01){
    
    F1.function = &tot;
    F1.params = &params;
    
    gsl_integration_qags(&F1, R, 40, 0, 1e-4, Nint, z, &result1, &error1); 
    //printf ("result  = % .18f\n", result1);
    
    re1 = result1;
    s = R/a_s;
    
    if (R < a_s-1.0e-9)
      {      	
	X_s = (1.0 / sqrt(1.0 - s*s)) * log((1.0+(sqrt(1.0-s*s)))/(s));
	I_R = (M_s/(2.0*M_PI*a_s*a_s*GAMMA*(1.0-s*s)*(1.0-s*s)))*((2.0+s*s)*X_s-3.0);
	sig_p = ((2.0*G*M_s*M_s*a_s)/(GAMMA*I_R*2.0*M_PI))*re1;
	printf("%16.8e\t %16.8e\n", R, I_R);
	//printf("sig_p^2(R) %6lf\n", sig_p);	
      }
    
  /*       if(R >= a_s-1.0e-10 && R <= a_s+1.0e-10)
	  {      	
	  X_s = 1.0;
	  I_R = 2.0*M_s/(15.0*M_PI*a_s*a_s*GAMMA);
	  sig_p = ((2.0*G*M_s*M_s*a_s)/(GAMMA*I_R*2.0*M_PI))*re1;
	  printf("%16.8e\t %16.8e\n", R, I_R);
	  //printf("sig_p^2(R) %6lf\n", sig_p);
	  }  */
       
    if (R > a_s+1.0e-9)
      {	       	  
	X_s = (1.0 / sqrt(s*s - 1.0)) * acos(1.0/s);
	I_R = (M_s/(2.0*M_PI*a_s*a_s*GAMMA*(1.0-s*s)*(1.0-s*s)))*((2.0+s*s)*X_s-3.0);
	sig_p = ((2.0*G*M_s*M_s*a_s)/(GAMMA*(I_R)*2.0*M_PI))*re1;
	printf("%16.8e\t %16.8e\n", R, I_R);
	//printf("sig_p^2(R) %6lf\n", sig_p);
	
      }      
    fprintf(mod,"%16.8e\t %16.8e\n", s, sig_p);
  }
  
  gsl_integration_workspace_free(z);
  
  fclose(mod);
  
  script = fopen( "script.gpl", "w" );
  fprintf(script, "set grid\nset terminal png\nset output 'sigma.png'\nset nokey\n");
  fprintf( script, "set title 'Sigma Proyectada vs R'\n" );
  fprintf( script, "plot 'model.dat' u 1:2 w l\n");
  fclose(script);
  
  warn = system("gnuplot script.gpl");
  
  return(warn);
}