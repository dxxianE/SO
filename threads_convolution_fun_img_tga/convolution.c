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

  int truncate = round(WIDTH/threadNum);

//Aplico el i*width+j
  for(int i = tid*truncate; i < tid*truncate+truncate; i++){
    for(int j = 0; j < HEIGHT; j++){
      //calculamos para i y j
      compute_target_pixel(i,j);
    }
  }
  free(args);
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
}

void multi_convolve_2(int n, int n_threads){
  for(int i = 0; i < n; i++){
    interchange();
    convolve_thread(n_threads);
  }
}
//-------------------- ----------------------  -------------------- 


int main(void) {
  // Allocate images
  pixels = malloc(WIDTH * HEIGHT);
  target = malloc(WIDTH * HEIGHT);

  load_image();

  // create a filter
  gaussian_filter();
  //vertical_filter();
  //horizontal_filter();

  load_image();
  multi_convolve_1(20);
  // write the convolved image
  write_tga("output.tga", target);

  //----------------- multi convolve con 1 hilo -----------------
  int one_th = 1;
  load_image();
  multi_convolve_2(10,one_th);
  //printf("· Multi_convolve_2, 10 iterations, 1 thread: %ld ms\n", endTimer(1));
  // write the convolved image
  write_tga("output_one_thread.tga", target);

  //----------------- multi convolve con n hilos ----------------
  int num_threads = 50;
  load_image();
  multi_convolve_2(20,num_threads);
  //printf("· Multi_convolve_1, 10 iterations, %d threads: %ld ms\n", num_threads, endTimer(2));
  // write the convolved image
  write_tga("output_multi_thread.tga", target);

  free(pixels);
  free(target);
  return 0;
}
 
