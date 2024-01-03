#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#define N 100
#define M 1000

/*
cada thread recibe dos idice, ver cual es mayor, si es asi cambiarlo
Dorian Erazo Orozco / 219395 / dorianemmanuel.erazo01@estudiant.upf.edu
*/

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#define N 100
#define M 1000

int a[N] = {0};  // Array de tamaño N para almacenar enteros
pthread_mutex_t lock[N];  // Array de mutex para sincronizar el acceso a elementos del array

// Función de ordenamiento que intercambia elementos del array
void* sort(int i, int j) {
    int tmp;

    for (; i < N; i++) {
        for (; j < N - i - 1; j++) {
            if (a[j] > a[j + 1]) {
                pthread_mutex_lock(&lock[j]);  // Bloquear el mutex antes de acceder a la sección crítica
                tmp = a[j];
                a[j] = a[j + 1];
                a[j + 1] = tmp;
                pthread_mutex_unlock(&lock[j]);  // Desbloquear el mutex después de acceder a la sección crítica
            }
        }
        j = 0;  // Reiniciar j después de cada iteración de i
    }
    pthread_exit(NULL);
}

// Función para transformar argumentos a enteros y llamar a la función de ordenamiento
void* trans_args_to_int(void* args) {
    int* arr = (int*)args;
    sort(arr[0], arr[1]);
    free(arr);
    return NULL;
}

int main() {
    pthread_t thread[M];  // Array de identificadores de hilo
    int i;

    // Generamos números aleatorios y los ponemos en el array 'a'
    for (int k = 0; k < N; k++) {
        int rand_num = rand() % 100;
        a[k] += rand_num;
        printf("\nA[%d]: %d", k, a[k]);
    }

    // Inicializamos mutex para cada elemento del array 'lock'
    for (i = 0; i < N; i++) {
        pthread_mutex_init(&lock[i], NULL);
    }

    // Creamos hilos con argumentos aleatorios para llamar a la función de ordenamiento
    for (i = 0; i < M; i++) {
        int* arr = malloc(2 * sizeof(int));
        arr[0] = rand() % N;

        do {
            arr[1] = rand() % N;
        } while (arr[0] == arr[1]);

        pthread_create(&thread[i], NULL, trans_args_to_int, arr);
    }

    printf("\nPrinting the sorted array:\n");
    for (i = 0; i < N; i++)
        printf("\n[%d]: %d", i, a[i]);

    // Destruimos mutex para cada elemento del array 'lock'
    for (i = 0; i < N; i++) {
        pthread_mutex_destroy(&lock[i]);
    }

    // Esperamos a que todos los hilos terminen
    for (i = 0; i < M; i++) {
        pthread_join(thread[i], NULL);
    }

    printf("\n");
    return 0;
}