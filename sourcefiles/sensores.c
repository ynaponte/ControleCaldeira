/* Monitor sensor, no arquivo sensor.c */ 
#include <math.h>
#include <pthread.h>
#include <string.h>

static pthread_mutex_t exclusao_mutua = PTHREAD_MUTEX_INITIALIZER; 
static pthread_cond_t alarme = PTHREAD_COND_INITIALIZER; 
static double s_temp = 0, s_nivel = 0, s_perturb = 0, s_ti = 0; 
static double limite_atual = HUGE_VAL;
static double nivel_max_atual = HUGE_VAL;
static double nivel_min_atual = 0;

/* Chamado pela thread que le o sensor e disponibiliza aqui o valor lido */
 void sensor_put( double temp, double nivel, double perturb, double ti) {
	 pthread_mutex_lock( &exclusao_mutua); 
	 s_temp = temp;
	 s_nivel = nivel;
	 s_perturb = perturb;
	 s_ti = ti;
	 if(s_temp >= limite_atual || s_nivel >= nivel_max_atual || s_nivel <= nivel_min_atual) 
	 	pthread_cond_signal( &alarme);
	 pthread_mutex_unlock( &exclusao_mutua); 
 }
 
 /* Chamado por qualquer thread que precisa do valor lido do sensor */ 
 double sensor_get(char s[5]) {
	 double aux; 
	 pthread_mutex_lock( &exclusao_mutua); 
	 if (strncmp(s,"t",1)==0)
	 aux = s_temp;
	 else if(strncmp(s,"h",1)==0)
	 aux = s_nivel;
	 else if(strncmp(s,"p",1)==0)
	 aux = s_perturb;
	 else if(strncmp(s,"i",1)==0)
	 aux = s_ti;
	 pthread_mutex_unlock( &exclusao_mutua); 
	 return aux;
 }

/* Thread fica bloqueada até o valor do sensor chegar em limite */ 
void sensor_alarme( double limite, double nivel_max, double nivel_min) {
	pthread_mutex_lock( &exclusao_mutua); 
	limite_atual = limite;
	nivel_max_atual = nivel_max;
	nivel_min_atual = nivel_min;
	while( s_temp < limite_atual && s_nivel < nivel_max_atual && s_nivel > nivel_min_atual) 
		pthread_cond_wait( &alarme, &exclusao_mutua); 
	limite_atual = HUGE_VAL;
	nivel_max_atual = HUGE_VAL; 
	nivel_min_atual = 0;
	pthread_mutex_unlock( &exclusao_mutua); 
}