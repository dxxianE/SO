#include <stdio.h>
#include <string.h>
#include <unistd.h>    // Unix-like system calls read and write
#include <fcntl.h>     // Unix-like system calls to open and close

#include "myutils.h"
#include <math.h>

#define WIDTH  1024
#define HEIGHT 1024
#define KLEN      7    // works with 3 or 7
#define KSIZE    49    // KLEN * KLEN

//int* image = malloc(WIDTH*HEIGHT);

#define MAX(a, b) (a < b ? b : a)
#define MIN(a, b) (a < b ? a : b)

unsigned char* pixels; // a pointer to the original image (needs malloc)
unsigned char* target; // a pointer to the target of convolution (needs malloc)

typedef struct kernel_struct {
  int values[KSIZE];
  int sum;
} kernel_type;

kernel_type filter; // the 3x3 kernel used to convolve the image

//---------------------- Barrera y funciones ----------------------
bool ended = false;

typedef struct barrier_struct{
  int n;
  //locks
  pthread_mutex_t lock;
  //condicion
  pthread_cond_t cond;
} barrier_type;

pthread_mutex_t lock;

void barrier_init(barrier_type* b, int i) {
  b->n = i;
  pthread_mutex_init(&b->lock, NULL);
  pthread_cond_init(&b->cond, NULL);
}

void barrier_wait_all(barrier_type* b){
  pthread_mutex_lock(&b->lock);
  b->n--;
  //printf("I've finished, now waiting for the others.\n");
  if(b->n > 0){
    pthread_cond_wait(&b->cond,&b->lock);
  }
  //printf("I'm the last one we can all go now\n");
  ended = true; // Activa el booleano para que un thread cambie la imagen 
  pthread_cond_broadcast(&b->cond);
  pthread_mutex_unlock(&b->lock);
}

void barrier_reset(barrier_type*b, int a){
  b->n = a;
}

barrier_type convolve_barrier;
barrier_type tga_barrier;

//----------------------------------------------------------------


static unsigned char tga[18];

// read a 1024x1024 image
void read_tga(char* fname) {
  int fd = open(fname, O_RDONLY);
  read(fd, tga, 18);
  read(fd, pixels, WIDTH * HEIGHT);
  close(fd);
}

// write a 1024x1024 image
void write_tga(char* fname, unsigned char *image) {
  int fd = open(fname, O_CREAT | O_RDWR, 0644);
  write(fd, tga, sizeof(tga));
  printf("Created file %s: Writing pixel size %d bytes\n", fname, WIDTH * HEIGHT);
  write(fd, image, WIDTH * HEIGHT);
  close(fd);
}

// create a random filter
void random_filter() {
  filter.sum = 0;
  for (int i = 0; i < KSIZE; i++) {
    filter.values[i] = rand() % 10;
    filter.sum += filter.values[i];
  }
}

void gaussian_3x3() {
  int values[] = { 1, 2, 1,
                   2, 4, 2,
                   1, 2, 1 };

  memcpy(filter.values, values, KSIZE * sizeof(int));

  filter.sum = 16;
}

void gaussian_7x7() {
  int values[] = { 0,  0,  1,   2,  1,  0, 0,
                   0,  3, 13,  22, 13,  3, 0,
                   1, 13, 59,  97, 59, 13, 1,
                   2, 22, 97, 159, 97, 22, 2,
                   1, 13, 59,  97, 59, 13, 1,
                   0,  3, 13,  22, 13,  3, 0,
                   0,  0,  1,   2,  1,  0, 0 };


  //void *memcpy(void *dest, const void *src, size_t n)
  //copies n characters from memory area src to memory area dest.
         //destino     //origen que va a ser  copy  //#bites copiados          
  memcpy(filter.values, values, KSIZE * sizeof(int));

  filter.sum = 0;
  for (int i = 0; i < KSIZE; i++)
    filter.sum += filter.values[i];
}

// create a Gaussian filter
void gaussian_filter() {
  if (KSIZE == 9) gaussian_3x3();
  else if (KSIZE == 49) gaussian_7x7();
}

// create a horizonal filter
void horizontal_filter() {
  for (int i = 0; i < KSIZE; i++) {
    int row = i / KLEN;
    if (3 * row < KLEN)
      filter.values[i] = -1;
    else if (KLEN < 2 * row)
      filter.values[i] = 1;
    else
      filter.values[i] = 0;
  }

  filter.sum = 0;
}

// create a vertical filter
void vertical_filter() {
  for (int i = 0; i < KSIZE; i++) {
    int col = i % KLEN;
    if (3 * col < KLEN)
      filter.values[i] = -1;
    else if (KLEN < 2 * col)
      filter.values[i] = 1;
    else
      filter.values[i] = 0;
  }

  filter.sum = 0;
}

void interchange() {
  unsigned char* aux = pixels;
  pixels = target;
  target = aux;
}

// compute the target pixel at (x,y)
void compute_target_pixel(int x, int y) {
  int i, j, sum = 0;
  int delta = (KLEN - 1) / 2;
  for (i = -delta; i <= delta; ++i)
    for (j = -delta; j <= delta; ++j)
      if (0 <= x + i && x + i < WIDTH && 0 <= y + j && y + j < HEIGHT)
        sum += filter.values[(i + delta) * KLEN + (j + delta)] * pixels[(x + i) * HEIGHT + (y + j)];

  if(filter.sum > 0) target[x * HEIGHT + y] = sum / filter.sum;
  else target[x * HEIGHT + y] = sum;
}

void convolve() {
  int x, y;
  for (x = 0; x < WIDTH; ++x)
    for (y = 0; y < HEIGHT; ++y)
      compute_target_pixel(x, y);
}

void multi_convolve_1(int iter) {
  // Add Code here
  for(int i = 0; i < iter; i++){
    interchange();
    convolve();
  }
}

void load_image() {
  read_tga("test.tga");  // read the image
  memcpy(target, pixels, WIDTH*HEIGHT);
}

//-------------------- OPERACIONES CON THREADS -------------------- 
void* convolveThread(void *args){
  int* th_id_aux = (int*)args;

  int tid = th_id_aux[0];
  int threadNum = th_id_aux[1];

  int rowss = round(WIDTH/threadNum);

//Aplico el i*width+j
  for(int i = tid*rowss; i < tid*rowss+rowss; i++){
    for(int j = 0; j < HEIGHT; j++){
      //calculamos para i y j
      compute_target_pixel(i,j);
    }
  }
  free(th_id_aux);
}

void convolve_thread(int th_num){
  pthread_t* tid;

  tid = malloc(th_num * sizeof(pthread_t));

  for(int i = 0; i < th_num; i++){
    //vamos a reservar un array de dos posiciones para poder
    //pasarle los datos al pthread_Create
    int* args = malloc(sizeof(int)*2);
    args[0] = i; //id del th
    args[1] = th_num; //el th
    pthread_create(&tid[i], NULL, convolveThread, (void*)args);
  }

  //tambien necesitaremos el join
  for(int k = 0; k < th_num; k++){
    pthread_join(tid[k], NULL);
  }
}

void multi_convolve_2(int n, int n_threads){
  for(int i = 0; i < n; i++){
    interchange();
    convolve_thread(n_threads);
  }
}

//-------------------- multi convolve con barrier --------------------
void* multiConvolveThread(void* param){
  int* aux = (int*) param; 

  int convol = (int)aux[0];
  int num_threads = (int)aux[1];
  int num_iter = (int)aux[2];

  //al igual que cuando calculamos threads sin barrera 
  //necesitaremos el numero de filas que tiene calcular un thread.
  int rows_b = WIDTH/num_threads;

  //esto seria la primera fila
  int initt = convol*rows_b;

  for(int i = 0; i < num_iter; i++){
    printf("Empezamos a esperar las barrier para la imagen\n");
    barrier_wait_all(&tga_barrier);

    for(int j= 0; j < WIDTH; j++){
      //printf("aqui el width, mira si hace bien el for\n");
      for(int k = initt; k < (initt+rows_b); k++){
        compute_target_pixel(j,k);
      }
    }
    printf("Thread id: %d, working on iteration: %d.\n",convol, num_iter);
    barrier_wait_all(&convolve_barrier);

    if(ended){
      ended = false;
      printf("Thread id: %d is change .tga\n.", convol);
      interchange();
      barrier_reset(&tga_barrier, num_threads);
      barrier_reset(&convolve_barrier, num_threads);
    }
  }
  free(aux);
}

void multi_convolve_3(int n, int n_threads){
  //un "array" para las id de los threads
  //pthread_t tids[n_threads];
  pthread_t* tids = malloc(sizeof(pthread_t)*n_threads);

  barrier_init(&convolve_barrier, n_threads);
  barrier_init(&tga_barrier, n_threads);

  for(int i = 0; i < n_threads; i++){
    int* create_threads = malloc(sizeof(int)*3);
    //En este caso i calcula el numero de rows que hay que calcular
    create_threads[0] = i;
    create_threads[1] = n_threads;
    //n son el numero de iteraciones gaussianas que hay que aplciar
    create_threads[2] = n;

    pthread_create(&tids[i], NULL, multiConvolveThread, create_threads);
  }
  for(int j = 0; j < n_threads; j++){
    pthread_join(tids[j],NULL);
  }
  free(tids);
}

/*
void multi_convolve_3(int n, int n_threads){
  //array de threads
  pthread_t tids[n_threads];
  //inicializamos la barrera
  barrier_init(&all_arrived, n_threads);

  for(int i = 0; i < n_threads; i++){
    //creamos una vez mas un array que contiene las 3 variables necesarias
    //en forma de un array.
    int* aux_3 = malloc(3*sizeof(int));
    aux_3[0] = i;
    aux_3[1] = n_threads;
    aux_3[2] = n; 

    pthread_create(&tids[i], NULL, multiConvolveThread, aux_3);
    free(aux_3);
  }
  for(int i = 0; i < n_threads; i++){
    pthread_join(tids[i], NULL);
  }
}
*/
//--------------------------------------------------------------------
int main(void) {
  // Allocate images
  pixels = malloc(WIDTH * HEIGHT);
  target = malloc(WIDTH * HEIGHT);

  load_image();

  // create a filter
  gaussian_filter();
  //vertical_filter();
  //horizontal_filter();
  startTimer(0);
  load_image();
  multi_convolve_1(20);
  // write the convolved image
  printf("----------------------------------------\n");
  write_tga("output.tga", target);
  printf("*Convolve estandar: %ld ms\n", endTimer(0));
  printf("----------------------------------------\n");
  //----------------- multi convolve con 1 hilo -----------------
  startTimer(0);
  int one_th = 1;
  load_image();
  multi_convolve_2(10,one_th);
  write_tga("output_one_thread.tga", target);
  printf("*Convolve con un hilo ha hecho: %ld ms\n", endTimer(0));
  printf("----------------------------------------\n");

  //----------------- multi convolve con n hilos ----------------
  startTimer(0);
  int num_threads = 16;
  load_image();
  multi_convolve_2(20,num_threads);
  write_tga("output_multi_thread.tga", target);
  printf("*Convolve con multi hilo ha hecho: %ld ms\n", endTimer(0));
  printf("----------------------------------------\n");

  //----------------- multi convolve con barrier -----------------
  startTimer(0);
  load_image();
  int n_threads = 8;
  multi_convolve_3(1, n_threads);
  write_tga("output_multi_barrier_thread.tga", target);  
  printf("*Convolve con barrier ha hecho: %ld ms\n", endTimer(0));
  printf("----------------------------------------\n");

  free(pixels);
  free(target);
  return 0;
}
 
