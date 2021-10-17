#ifdef _WIN32
#include <windows.h> //import windows.h to change console color
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

//color codes
#define RED 31
#define GREEN 32
#define YELLOW 33
#define BLUE 34
#define RESET 0
//constants
#define MIN_KERNEL_SIZE 3
#define LINE_SIZE 100
#define PADDING_VALUE 0
#define MAX_PIXEL_VAL 255
#define BRAND_NAME "Besher"

//generic functions
void setColor(int);
void removeExtension(char*, char*);
bool isSpace(char);

//generic PGM functions
bool isBinaryPgm(FILE*);
bool isAsciiPgm(FILE*);
void skipComments(FILE*);
bool writeArrToPgm(uint8_t*, int, int, char*, int);

//program-specific functions
uint8_t* allocArrWithPad(int, int, int);
size_t fwritePixels(const void*, size_t, size_t, FILE*);
uint8_t* rBinaryPgmPad(char*, int*, int*, int);
int* applyKernelArr(int*, int, float, uint8_t*, int, int);
int applyKernelPix(int*, int, float, uint8_t*, int, size_t, size_t);
uint8_t* filterScale(int*, int, int);
uint8_t* filterMinMax(int*, int, int);

int main(int argc, char const *argv[]) {
  uint8_t* arr, *pixelValues;
  int width, height, *arrValues;
  int kernel[9] = {-1, 0, 1, -1, 0, 1, -1, 0, 1};
  arr = rBinaryPgmPad(argv[1], &width, &height, 3);
  arrValues = applyKernelArr(kernel, 3, 1, arr, width+2, height+2);
  free(arr);
  pixelValues = filterMinMax(arrValues, width, height);
  free(arrValues);
  writeArrToPgm(pixelValues, width, height, argv[2], 2);
  free(pixelValues);
  return 0;
}

uint8_t* rBinaryPgmPad(char* fileName, int* width, int* height, int kernelSize) {
  FILE* picture;
  uint8_t* pixelValues;
  size_t i, j;
  int numberHolder;
  int padding = kernelSize>>1;
  if (kernelSize < MIN_KERNEL_SIZE && kernelSize % 2) {
    setColor(RED);
    printf("Error: kernel size must be >= %d and odd", MIN_KERNEL_SIZE);
    setColor(RESET);
  }
  if (!(picture = fopen(fileName, "rb"))) {
    setColor(RED);
    printf("Couldn't read %s\nProbably doesn't exist\n", fileName);
    setColor(RESET);
    return NULL;
  }
  if (!isBinaryPgm(picture)) {
    setColor(RED);
    printf("%s is not a binary PGM file\n", fileName);
    setColor(RESET);
    fclose(picture);
    return NULL;
  }
  skipComments(picture);
  char* lineHolder = (char*) malloc(LINE_SIZE * sizeof(char));
  fscanf(picture, "%d %d", width, height);
  // get maximum value
  skipComments(picture);
  fscanf(picture, "%d", &numberHolder);
  fgetc(picture);
  free(lineHolder);
  // rowWidth is the width plus the padding on both sides
  int rowWidth = *width + padding*2;
  pixelValues = allocArrWithPad(*width, *height, kernelSize);
  if (!pixelValues) {
    setColor(RED);
    printf("%s:%d > Failed to allocate memory\n", __FILE__, __LINE__);
    setColor(RESET);
    return NULL;
  }
  //initialize padding rows with 0
  for (j=0, i=0; j < padding; i++, j++)
    memset(pixelValues + i*rowWidth, PADDING_VALUE, rowWidth);
  //set pixelvalues adding padding at the beginning and end of each row
  for (j=0; j < *height; i++, j++) {
    memset(pixelValues + i*rowWidth, PADDING_VALUE, padding);
    fread(pixelValues + i*rowWidth + padding, sizeof(uint8_t), (*width), picture);
    memset(pixelValues + i*rowWidth + padding + *width, PADDING_VALUE, padding);
  }
  //initialize last padding rows with 0
  for (j=0; j < padding; i++,j++)
    memset(pixelValues + i*rowWidth, PADDING_VALUE, rowWidth);
  fclose(picture);
  return pixelValues;
}

uint8_t* allocArrWithPad(int width, int height, int kernelSize) {
  // divide by 2 (floor)
  int padding = kernelSize>>1;
  printf("allocating %d bytes\n", (width+padding*2)*(height+padding*2));
  return (uint8_t*) malloc((width+padding*2)*(height+padding*2));
}

void removeExtension(char* filename, char* stripped) {
  strcpy(stripped, filename);
  char *end = stripped + strlen(stripped);
  while (end > stripped && *end != '.' && *end != '\\' && *end != '/') {
    end--;
  }
  if ((end > stripped && *end == '.') &&
      (*(end - 1) != '\\' && *(end - 1) != '/')) {
      *end = '\0';
  }
}

int* applyKernelArr(int* kernel, int kernelSize, float coefficient, uint8_t* pixelValues, int width, int height) {
  int* outputArr;
  int padding, areaWidth, areaHeight;
  size_t i, j;
  padding = kernelSize >> 1;
  areaWidth = width - padding*2;
  areaHeight = height - padding*2;
  outputArr = (int*) malloc(areaWidth * areaHeight * sizeof(int));
  for (i = 0; i < areaHeight; i++) {
    for (j = 0; j < areaWidth; j++)
      outputArr[i*areaWidth+j] = applyKernelPix(kernel, kernelSize, coefficient, pixelValues, width, i+padding, j+padding);
  }
  return outputArr;
}

int applyKernelPix(int* kernel, int kernelSize, float coefficient, uint8_t* pixelValues, int width, size_t xLoc, size_t yLoc) {
  size_t i, j;
  int result = 0, kernelPadding, row;
  kernelPadding = kernelSize >> 1;
  for (i = 0; i < kernelSize; i++) {
    row = xLoc+i-kernelPadding;
    for (j = 0; j < kernelSize; j++){
      result += pixelValues[(yLoc+j-kernelPadding)+row*width] * kernel[i*kernelSize+j];
    }
  }
  return (int)((float)(result) * coefficient);
}

uint8_t* filterScale(int* arrValues, int width, int height) {
  size_t i;
  uint8_t* outputPixValues = (uint8_t*) malloc(width * height * sizeof(uint8_t));
  for (i = 0; i < height*width; i++) {
    outputPixValues[i] = (uint8_t)(arrValues[i] < 0 ? 0 : (arrValues[i] > MAX_PIXEL_VAL ? MAX_PIXEL_VAL : arrValues[i]));
  }
  return outputPixValues;
}

uint8_t* filterMinMax(int* arrValues, int width, int height) {
  size_t i;
  int targetMin = 0, targetMax = 255, srcMin, srcMax, srcScale, targetScale, src, scaled;
  float scaleRatio;
  srcMax = srcMin = arrValues[0];
  uint8_t* outputPixValues = (uint8_t*) malloc(width * height * sizeof(uint8_t));
  for (i = 1; i < height*width; i++) {
    if (arrValues[i] < srcMin)
      srcMin = arrValues[i];
    if (arrValues[i] > srcMax)
      srcMax = arrValues[i];
  }
  printf("MIN:%d MAX:%d\n", srcMin, srcMax);
  if (srcMax == srcMin) {
    if (srcMin < 255 && srcMin > 0) {
      for (i = 0; i < width*height; i++) {
        outputPixValues[i] = arrValues[i];
      }
    } else if (srcMin > 255) {
      memset(outputPixValues, 255, width*height);
    } else {
      memset(outputPixValues, 0, width*height);
    }
    return outputPixValues;
  }
  srcScale = srcMax - srcMin;
  targetScale = targetMax - targetMin;
  scaleRatio = (float)(targetScale) / (float)(srcScale);
  for (i = 0; i < height*width; i++) {
    src = arrValues[i] - srcMin;
    scaled = src * scaleRatio;
    outputPixValues[i] = (uint8_t)(scaled + targetMin);
  }
  return outputPixValues;
}

bool isBinaryPgm(FILE* file) {
  char* typeHolder = (char*) malloc(LINE_SIZE * sizeof(char));
  fscanf(file, " %s ", typeHolder);
  if (strcmp(typeHolder, "P5")) {
    free(typeHolder);
    return false;
  }
  free(typeHolder);
  return true;
}

bool isAsciiPgm(FILE* file) {
  char* typeHolder = (char*) malloc(LINE_SIZE * sizeof(char));
  fscanf(file, " %s ", typeHolder);
  if (strcmp(typeHolder, "P2")) {
    free(typeHolder);
    return false;
  }
  free(typeHolder);
  return true;
}

bool writeArrToPgm(uint8_t* pixelValues, int width, int height, char* outputName, int version) {
  FILE* picture;
  size_t (*writepixPtr)(const void*, size_t, size_t, FILE*) = fwrite;
  if (!(picture = fopen(outputName, "wb"))) {
    setColor(RED);
    printf("Cannot create %s\n", outputName);
    setColor(RESET);
    return false;
  }
  fprintf(picture, "P%d\n",version);
  if (version == 2) {
    writepixPtr = fwritePixels;
  }
  fprintf(picture, "# decompressed by %s\n", BRAND_NAME);
  fprintf(picture, "%d %d\n", width, height);
  fprintf(picture, "255\n");
  writepixPtr(pixelValues, sizeof(uint8_t), width * height, picture);
  fclose(picture);
  setColor(GREEN);
  printf("Wrote to output %s Successfully\n", outputName);
  setColor(RESET);
  return true;
}

size_t fwritePixels(const void *ptr, size_t size, size_t nmemb, FILE *picture) {
  size_t i;
  for (i = 0; i < nmemb; i++) {
    fprintf(picture, "%u\n", ((unsigned char*)ptr)[i]);
  }
  return i;
}

bool isSpace(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}

void skipComments(FILE* file) {
  int c;
  char* lineHolder = (char*) malloc(LINE_SIZE * sizeof(char));
  while ((c = fgetc(file)) && isSpace(c));
  if (c == '#') {
    fgets(lineHolder, LINE_SIZE, file);
    skipComments(file);
  } else {
    fseek(file, -1, SEEK_CUR);
  }
  free(lineHolder);
}

//functions that are defined differently based on platform
#ifdef _WIN32
void setColor(int color) {
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  switch (color) {
    case 31:
      SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
    break;
    case 32:
      SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);
    break;
    case 33:
      SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
    break;
    case 34:
      SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE);
    break;
    default:
      SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
  }
}
#else
void setColor(int color) {
  printf("\x1b[%dm", color);
}
#endif
