//Defini��o de Bibliotecas
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "socket.h"
#include "sensores.h"
#include "tela.h"
#include "bufduplo.h"
#include "bufduploTH.h"
#include "referenciaT.h"
#include "referenciaH.h"

#define	NSEC_PER_SEC    (1000000000) 	// Numero de nanosegundos em um segundo

#define N_AMOSTRAS 10000

void thread_mostra_status (void){
	double t, h;
	while(1){
		t = sensor_get("t");
		h = sensor_get("h");
		aloca_tela();
		system("tput reset");
		printf("---------------------------------------\n");
		printf("Temperatura (T)--> %.2lf\n", t);
		printf("Nivel       (H)--> %.2lf\n", h);
		printf("---------------------------------------\n");
		libera_tela();
		sleep(1);
		//								
	}	
		
}


void thread_le_sensor (void){ //Le Sensores periodicamente a cada 10ms
	struct timespec t;
	long periodo = 10e6; //10e6ns ou 10ms
	
	// Le a hora atual, coloca em t
	clock_gettime(CLOCK_MONOTONIC ,&t);
	while(1){
		// Espera ateh inicio do proximo periodo
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
		
		//Envia mensagem via canal de comunica��o para receber valores de sensores		
		sensor_put(msg_socket("st-0"), msg_socket("sh-0"), msg_socket("sno0"), msg_socket("sti0"));
		
		// Calcula inicio do proximo periodo
		t.tv_nsec += periodo;
		while (t.tv_nsec >= NSEC_PER_SEC) {
			t.tv_nsec -= NSEC_PER_SEC;
			t.tv_sec++;
		}		
	}		
}

void thread_alarme (void){
	while(1){
		sensor_alarme(39.0, 3.0, 0.1);
		aloca_tela();
		printf("ALARME\n");
		libera_tela();
		sleep(1);	
	}
		
}

///Controle
void thread_controle_geral (void){
	char msg_enviada[1000];
	long atraso_fim;
	struct timespec t, t_fim;
	long periodo = 50e6; //50ms
	double temp, ref_temp, nivel, ref_nivel, delta_rel, perturb, ti;
	double comp_Q;

	// Le a hora atual, coloca em t
	clock_gettime(CLOCK_MONOTONIC ,&t);
	t.tv_sec++;
	while(1){
		
		// Espera ateh inicio do proximo periodo
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
		
		temp = sensor_get("t");
		nivel = sensor_get("h");
		perturb = sensor_get("p");
		ti = sensor_get("i");
		ref_temp = ref_getT();
		ref_nivel = ref_getH();

		delta_rel = (ref_nivel-nivel)/ref_nivel;

		double na, ni;

    	if(temp > ref_temp) { //diminui temperatura

			sprintf( msg_enviada, "ani%lf", (100.0*(1 + delta_rel) + perturb/2));
	        msg_socket(msg_enviada);
	        
			sprintf( msg_enviada, "anf%lf", (100.0*(1 - delta_rel) - perturb/2));
	        msg_socket(msg_enviada);
			
			sprintf( msg_enviada, "ana%lf", 0.0 );
			msg_socket(msg_enviada);

			sprintf( msg_enviada, "aq-%lf", 10000.0);
			msg_socket(msg_enviada);
        }

        
        if(temp < ref_temp) {    //aumenta temperatura

	        if((ref_temp-temp)*20>10.0){
				na = 10.0;  // Valor máximo suportado pela entrada de água quente
				ni = perturb + 300*delta_rel; // compensa perturbação de saida
			}	
	        else
			{
	        	na = (ref_temp-temp)*20;  // Compensa temperatura com agua quente
				ni = (10.0 - na + perturb) + 300*delta_rel;  // Compensa perturbações e diferenças de nível
			}		
			// Compensa o calor perdido pela inserção de agua fria
			comp_Q = 10000.0 - ni*(ti - temp)*4184;

			sprintf( msg_enviada, "ani%lf", ni);
	        msg_socket(msg_enviada);
			
			sprintf( msg_enviada, "anf%lf", 10.0);
	        msg_socket(msg_enviada);
			
	        sprintf( msg_enviada, "ana%lf", na);
			msg_socket(msg_enviada);

			sprintf( msg_enviada, "aq-%lf", comp_Q);
			msg_socket(msg_enviada);
        }
        
		// Le a hora atual, coloca em t_fim
		clock_gettime(CLOCK_MONOTONIC ,&t_fim);
		
		// Calcula o tempo de resposta observado em microsegundos
		atraso_fim = 1000000*(t_fim.tv_sec - t.tv_sec)   +   (t_fim.tv_nsec - t.tv_nsec)/1000;
		
		bufduplo_insereLeitura(atraso_fim);
		bufduplo_insereLeituraTH(temp*1000);
		bufduplo_insereLeituraTH(nivel*1000);
		
		// Calcula inicio do proximo periodo
		t.tv_nsec += periodo;
		while (t.tv_nsec >= NSEC_PER_SEC) {
			t.tv_nsec -= NSEC_PER_SEC;
			t.tv_sec++;
		}
	}
}

void thread_grava_temp_resp(void){
	FILE* dados_f;
	dados_f = fopen("dados.txt", "w");
    if(dados_f == NULL){
        printf("Erro, nao foi possivel abrir o arquivo\n");
        exit(1);    
    }
	int amostras = 1;	
	while(amostras++<=N_AMOSTRAS/200){
		long * buf = bufduplo_esperaBufferCheio();		
		int n2 = tamBuf();
		int tam = 0;
		while(tam<n2)
			fprintf(dados_f, "%4ld\n", buf[tam++]); 
		fflush(dados_f);
		aloca_tela();
		printf("Gravando no arquivo...\n");
		
		libera_tela();
						
	}
	
	fclose(dados_f);	
}

void thread_grava_temp_nivel(void){
	FILE* dados_f;
	dados_f = fopen("dados_t_h.txt", "w");
    if(dados_f == NULL){
        printf("Erro, nao foi possivel abrir o arquivo\n");
        exit(1);    
    }
	int amostras = 1;	
	while(amostras++<=N_AMOSTRAS/200){
		long * buf = bufduplo_esperaBufferCheioTH();		
		int n2 = tamBufTH();
		int tam = 0;
		while(tam<n2)
			fprintf(dados_f, "%4ld\n", buf[tam++]); 
		fflush(dados_f);
		aloca_tela();
		printf("Gravando no arquivo...\n");
		
		libera_tela();
						
	}
	
	fclose(dados_f);	
}

int main( int argc, char *argv[]) {
    ref_putT(29.0);
	ref_putH(2.0);
	cria_socket(argv[1], atoi(argv[2]) );
    
	pthread_t t1, t2, t3, t4, t5, t6;
    
    pthread_create(&t1, NULL, (void *) thread_mostra_status, NULL);
    pthread_create(&t2, NULL, (void *) thread_le_sensor, NULL);
    pthread_create(&t3, NULL, (void *) thread_alarme, NULL);
    pthread_create(&t4, NULL, (void *) thread_controle_geral, NULL);
    pthread_create(&t5, NULL, (void *) thread_grava_temp_resp, NULL);
	pthread_create(&t6, NULL, (void *) thread_grava_temp_nivel, NULL);
    
	pthread_join( t1, NULL);
	pthread_join( t2, NULL);
	pthread_join( t3, NULL);
	pthread_join( t4, NULL);
	pthread_join( t5, NULL);
	pthread_join( t6, NULL);
	
	return 0;
	    
}
