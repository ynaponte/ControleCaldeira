#include <math.h>
#include <pthread.h>

static pthread_mutex_t exclusao_mutua = PTHREAD_MUTEX_INITIALIZER;
static double ref_entrada = 0;

void ref_putH(double ref) {
    pthread_mutex_lock(&exclusao_mutua);
    ref_entrada = ref;
    pthread_mutex_unlock(&exclusao_mutua);
}

double ref_getH(void) {
    double aux;
    pthread_mutex_lock(&exclusao_mutua);
    aux = ref_entrada;
    pthread_mutex_unlock(&exclusao_mutua);
    return aux;
}