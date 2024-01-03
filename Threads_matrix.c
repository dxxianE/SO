/*Establezca cada entrada de un arreglo 2D A[100][50] en la suma de sus 
índices A[i][j] = i + j de la siguiente manera: 
(1) inicialícelo a 0 (para todo A[i][j] = 0), 
(2) crear 100 subprocesos de fila, cada uno sumará sus  índices a todos los elementos de su fila. 
(3) crear 50 subprocesos de columna, cada uno sumará sus índices a todos los elementos de su columna. 
Discuta la necesidad de candados; ¿Es mejor usar un solo candado o es mejor usar 
muchos candados?*/

//-pthread .c .c

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define ROW 100
#define COL 50

int matrix[ROW][COL] = {0};  // Matriz 2D de tamaño ROW x COL
pthread_mutex_t lock;  // Mutex para proteger la sección crítica

// Función para realizar cálculos en la matriz (para filas o columnas)
void* calculus(void* th_id){
    int id = *((int*)th_id);  // Obtener el identificador del hilo

    for (int i = 0; i < COL; i++){
        pthread_mutex_lock(&lock);  // Bloquear el mutex antes de acceder a la sección crítica
        matrix[id][i] = id + i;  // Realizar el cálculo en la matriz
        pthread_mutex_unlock(&lock);  // Desbloquear el mutex después de acceder a la sección crítica
    }

    free(th_id);  // Liberar la memoria asignada al identificador del hilo
    pthread_exit(NULL);  // Terminar el hilo
}

// Función para inicializar valores en una fila de la matriz
void* calculusRow(void* th_id_row){
    int r = *((int*)th_id_row);  // Obtener el identificador de la fila

    for (int i = 0; i < COL; i++){
        matrix[r][i] = r;  // Inicializar la fila con el valor de la fila
    }

    free(th_id_row);  // Liberar la memoria asignada al identificador de la fila
    pthread_exit(NULL);  // Terminar el hilo
}

// Función para inicializar valores en una columna de la matriz
void* calculusCol(void* th_id_col){
    int c = *((int*)th_id_col);  // Obtener el identificador de la columna

    for (int i = 0; i < ROW; i++){
        matrix[i][c] = c;  // Inicializar la columna con el valor de la columna
    }

    free(th_id_col);  // Liberar la memoria asignada al identificador de la columna
    pthread_exit(NULL);  // Terminar el hilo
}

int main(){
    pthread_t tidRow[ROW];  // Arreglo de identificadores de hilos para filas
    pthread_t tidCol[COL];  // Arreglo de identificadores de hilos para columnas

    pthread_mutex_init(&lock, NULL);  // Inicializar el mutex

    // Crear hilos para inicializar valores en filas
    for (int i = 0; i < ROW; i++){
        int* th_id_row = malloc(sizeof(int));
        *th_id_row = i;
        printf("Creating thread for row: %d\n", *th_id_row);
        pthread_create(&tidRow[i], NULL, calculusRow, (void*)th_id_row);
    }

    // Crear hilos para inicializar valores en columnas
    for (int j = 0; j < COL; j++){
        int* th_id_col = malloc(sizeof(int));
        *th_id_col = j;
        printf("Creating thread for column: %d\n", *th_id_col);
        pthread_create(&tidCol[j], NULL, calculusCol, (void*)th_id_col);
    }

    // Esperar a que todos los hilos de filas terminen
    for (int i = 0; i < ROW; i++){
        pthread_join(tidRow[i], NULL);
    }

    // Esperar a que todos los hilos de columnas terminen
    for (int j = 0; j < COL; j++){
        pthread_join(tidCol[j], NULL);
    }

    pthread_mutex_destroy(&lock);  // Destruir el mutex después de su uso

    // Imprimir la matriz resultante para verificar los resultados
    printf("\nResulting Matrix:\n");
    for (int i = 0; i < ROW; i++){
        for (int j = 0; j < COL; j++){
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }

    return 0;
}