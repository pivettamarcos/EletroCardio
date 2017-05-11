#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define NUM_SENSORES 6
#define NUM_RESULTADOS_COMPLETOS 24
#define NUM_RESULTADOS_SUMARIZADOS 10

bool flag_completo = false;
int x, y, z;
int par1_sensores[2];
int par2_sensores[2];
int cont_pos = -1;

int resultados[NUM_RESULTADOS_COMPLETOS][3];
long double resultados_sumarizados[NUM_RESULTADOS_SUMARIZADOS];

int pares_especificos[10] = {2,3,4,5,6,8,10,12,15,20};
char* nome_sensores[10] = {"1 e 2", "1 e 3", "1 e 4", "1 e 5", "2 e 3", "2 e 4", "2 e 5", "3 e 4", "3 e 5", "4 e 5"};

int rand_numb1, rand_numb2;

struct input{
	int indices[2];
	int valores[2];
};

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void mostra_opcoes_iniciais();
int gera_dados_sensor(int i);
void sumarizador();
void gera_estado_aleatorio();
void* thread_coleta(void *arg);
void* imprime_resultados();

int main(void){
	pthread_t par1;
	pthread_t par2;
	
	mostra_opcoes_iniciais();

	while(1){
		gera_estado_aleatorio();

		struct input *entrada = malloc(sizeof(struct input));

		entrada->indices[0] = par1_sensores[0];
		entrada->indices[1] = par1_sensores[1];

		entrada->valores[0] = gera_dados_sensor(par1_sensores[0]);
		entrada->valores[1] = gera_dados_sensor(par1_sensores[1]);
	
		pthread_create(&par1, NULL, thread_coleta, entrada);

		struct input *entrada2 = malloc(sizeof(struct input));

		entrada2->indices[0] = par2_sensores[0];
		entrada2->indices[1] = par2_sensores[1];

		entrada2->valores[0] = gera_dados_sensor(par2_sensores[0]);
		entrada2->valores[1] = gera_dados_sensor(par2_sensores[1]);
	
		pthread_create(&par2, NULL, thread_coleta, entrada2);

		pthread_join(par1, NULL);
		pthread_join(par2, NULL);
	}
}

void mostra_opcoes_iniciais(){
	printf("Você deseja ver os resultados de maneira completa? S/N\n");
	char resposta;
	resposta = getchar();

	while(resposta != 'S' && resposta != 'N'){
		printf("Caracter inválido, insira S ou N\n");
		resposta = getchar();
	}
	if(resposta == 'S')
		flag_completo = true;
}

int gera_dados_sensor(int i){

	switch(i){
		case 0:
			return 1 + rand() % 98;
		case 1:
			return 1 + rand() % 356;
		case 2:
			return 1 + rand() % 745;
		case 3:
			return 1 + rand() % 48;
		case 4:
			return 1 + rand() % 3781;
	}
}

void* thread_coleta(void *arg){

	struct input *entrada = arg;

	//INICIO SEÇÃO CRÍTICA
	
	pthread_mutex_lock(&mutex);	

	cont_pos++;

	pthread_mutex_unlock(&mutex);

	//FIM SEÇÃO CRÍTICA

	if(cont_pos >= NUM_RESULTADOS_COMPLETOS){
		sumarizador();
		cont_pos = 0;
		pthread_t mostra;
		pthread_create(&mostra, NULL, imprime_resultados, NULL);
		pthread_join(mostra,NULL);
	}

	resultados[cont_pos][0] = entrada->valores[0] * entrada->valores[1];
	resultados[cont_pos][1] = entrada->indices[0];
	resultados[cont_pos][2] = entrada->indices[1];

	pthread_exit(NULL);
}

void sumarizador(){
	int x = 0, y = 0;
	long int medias[NUM_RESULTADOS_SUMARIZADOS];
	int cont_pares[NUM_RESULTADOS_SUMARIZADOS];

	for(x = 0; x < NUM_RESULTADOS_SUMARIZADOS; x++){
		resultados_sumarizados[x] = 0.0;
		medias[x] = 0;
		cont_pares[x] = 0;
	}

	for(x = 0; x < NUM_RESULTADOS_COMPLETOS; x++){
		int resultado_multiplicacao = (resultados[x][1] + 1) * (resultados[x][2] + 1);

		for(y = 0; y < NUM_RESULTADOS_SUMARIZADOS; y++){				
			if(resultado_multiplicacao == pares_especificos[y]){
				medias[y] += resultados[x][0];
				cont_pares[y]++;
			}
		}
	}
	
	for(x = 0; x < NUM_RESULTADOS_SUMARIZADOS; x++){
		if(cont_pares[x] != 0)
			resultados_sumarizados[x] = medias[x]/cont_pares[x];
	}
}

void gera_estado_aleatorio(){
	rand_numb1 = rand() % 5;

	bool flag = false;

	while(!flag){
		rand_numb2 = rand() % 5;
		if(rand_numb2 != rand_numb1){
			flag = true;
		}
	}

	par1_sensores[0] = rand_numb1;
	par1_sensores[1] = rand_numb2;

	flag = false;
		
	while(!flag){
		rand_numb1 = rand() % 5;
		if(rand_numb1 != par1_sensores[0] && rand_numb1 != par1_sensores[1]){
			flag = true;
		}
	}

	flag = false;

	while(!flag){
		rand_numb2 = rand() % 5;
		if(rand_numb2 != rand_numb1 && rand_numb2 != par1_sensores[0] && rand_numb2 != par1_sensores[1]){
			flag = true;
		}
	}
	
	par2_sensores[0] = rand_numb1;
	par2_sensores[1] = rand_numb2;
}

void* imprime_resultados(){
	printf("\n\n=======================================================================\n\n");
	
	if(flag_completo){
		for(z = 0; z < NUM_RESULTADOS_COMPLETOS; z++){
			printf("%d | SENSOR %d X SENSOR %d - RESULTADO : %d\n", z, resultados[z][1] + 1, resultados[z][2] + 1, resultados[z][0]);
			if(z%2 != 0 && z != 0){
				printf("-----------------------------------------------------------\n");
				sleep(1);
			}
		}
	}

	for(x = 0; x < NUM_RESULTADOS_SUMARIZADOS; x++){
		printf("|Sensores %s - RESULTADO: %.2Lf\n", nome_sensores[x], resultados_sumarizados[x]);
	}
	sleep(2);

	printf("\n=======================================================================\n\n");
	
	pthread_exit(NULL);
}
