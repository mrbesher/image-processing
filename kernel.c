#ifdef _WIN32
#include <windows.h> //import windows.h to change console color
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

//color codes
#define RED 31
#define GREEN 32
#define YELLOW 33
#define BLUE 34
#define RESET 0
//constants
#define MIN_KERNEL_SIZE 3
#define LINE_SIZE 100
#define INPUT_SIZE 500
#define MAX_ARG_NUMBER 5
#define PADDING_VALUE 0
#define MAGIC_NUMBER_SIZE 3
#define MAX_PIXEL_VAL 255
#define BRAND_NAME "Besher"

typedef struct {
  int size;
  int* matrix;
} mask;

//generic functions
void setColor(int);
void removeExtension(char*, char*);
bool isSpace(char);
int* staticToDynamicKernel(int[], int);
void help();

//generic PGM functions
bool isBinaryPgm(FILE*);
bool isAsciiPgm(FILE*);
void skipComments(FILE*);
bool writeArrToPgm(uint8_t*, int, int, char*, int);

//program-specific functions
uint8_t* allocArrWithPad(int, int, int);
size_t fwritePixels(const void*, size_t, size_t, FILE*);
uint8_t* rBinaryPgmPad(char*, int*, int*, int);
uint8_t* rAsciiPgmPad(char*, int*, int*, int);
int* applyKernelArr(int*, int, float, uint8_t*, int, int);
int applyKernelPixWithCo(int*, int, float, uint8_t*, int, size_t, size_t);
int applyKernelPixNoCo(int*, int, float, uint8_t*, int, size_t, size_t);
uint8_t* filterSlice(int*, int, int);
uint8_t* filterMinMax(int*, int, int);
void setPaddingMirror(uint8_t*, int, int, int);
uint8_t* addStaticPad(uint8_t*, uint8_t, int, int, int);
bool doesKernelFit(int, int, int);
bool processInput(char*);

//kernel-specific functions
int applyVerPrewittPgm(char*, char*);
int* applyPrewittVertical(uint8_t*, int, int, bool);
int* applyPrewittHorizontal(uint8_t*, int, int, bool);
int applySobelPgm(char*, char*);

int main(int argc, char const *argv[]) {
  char* inputStr = (char*) malloc(INPUT_SIZE * sizeof(char));
  printf("Welcome to Stipant\n");
  help();
  do {
    printf("> ");
    fgets(inputStr, INPUT_SIZE, stdin);
  } while(processInput(inputStr));
  free(inputStr);
  return 0;
}

void help() {
  printf(
          "Here are the commands you can use:\n"
          "help\t\t\t\t\t- prints available commands\n"
          "verprewitt input.pgm [output.pgm]\t- applies prewitt vertical operator to input.pgm\n"
          "sobel input.pgm [output.pgm]\t\t- applies sobel filter to input.pgm\n"
          "exit\t\t\t\t\t- quits the program\n"
        );
}

bool processInput(char* inputStr) {
  int i;
  char* strPointer;
  bool shouldCont = true;
  char** arg = (char**) malloc(MAX_ARG_NUMBER * sizeof(char*));
  for (i = 0; i < MAX_ARG_NUMBER; i++) {
    arg[i] = (char*) malloc(LINE_SIZE * sizeof(char));
    strcpy(arg[i],"NULL");
  }
  i = 0;
  strPointer = strtok(inputStr, " ");
  strcpy(arg[i++], strPointer);
  while ((strPointer = strtok(NULL, " "))) {
    strcpy(arg[i++], strPointer);
  }
  strtok(arg[i -1], "\n");
  i=0;
  if (!strcmp(arg[i], "exit")) {
    shouldCont = false;
  } else if (!strcmp(arg[i], "help")) {
    help();
  } else if (!strcmp(arg[i], "verprewitt")) {
    i++;
    if (strcmp(arg[i++], "NULL")) {
      if (strcmp(arg[i], "NULL")) {
        applyVerPrewittPgm(arg[i-1], arg[i]);
      } else {
        removeExtension(arg[i-1], arg[i]);
        strcat(arg[i], "_verprewitt.pgm");
        applyVerPrewittPgm(arg[i-1], arg[i]);
      }
    } else {
      setColor(RED);
      printf("Error: no input provided\n");
      setColor(YELLOW);
      printf("Tip: type 'help' for usage\n");
      setColor(RESET);
    }
  } else if (!strcmp(arg[i], "sobel")) {
    i++;
    if (strcmp(arg[i++], "NULL")) {
      if (strcmp(arg[i], "NULL")) {
        applySobelPgm(arg[i-1], arg[i]);
      } else {
        removeExtension(arg[i-1], arg[i]);
        strcat(arg[i], "_sobel.pgm");
        applySobelPgm(arg[i-1], arg[i]);
      }
    } else {
      setColor(RED);
      printf("Error: no input provided\n");
      setColor(YELLOW);
      printf("Tip: type 'help' for usage\n");
      setColor(RESET);
    }
  } else {
    setColor(YELLOW);
    printf("Cannot recognise command '%s'\n", arg[i]);
    printf("Tip: type 'help' for usage\n");
    setColor(RESET);
  }

  for (i = 0; i < MAX_ARG_NUMBER; i++) {
    free(arg[i]);
  }
  free(arg);
  return shouldCont;
}

int applySobelPgm(char* inputFileName, char* outputFileName) {
  uint8_t *arr, *pixelValues;
  int width, height, widthNoPad, heightNoPad, xDirValue, yDirValue, *verPrewittArr,
      *horPrewittArr, *filteredIntegers, padding, kernelSize = 3;
  size_t i, j;
  arr = rBinaryPgmPad(inputFileName, &width, &height, 0);
  if (!arr) {
    setColor(YELLOW);
    printf("Trying ASCII pgm format...\n");
    setColor(RESET);
    arr = rAsciiPgmPad(inputFileName, &width, &height, 0);
  }
  if (!arr)
    return 1;
  verPrewittArr = applyPrewittVertical(arr, width, height, true);
  horPrewittArr = applyPrewittHorizontal(arr, width, height, true);
  free(arr);
  padding = (kernelSize>>1);
  widthNoPad = width-padding*2;
  heightNoPad = height-padding*2;
  filteredIntegers = (int*) malloc(widthNoPad*heightNoPad*sizeof(int));
  for (i = 0; i < heightNoPad; i++) {
    for (j = 0; j < widthNoPad; j++) {
      xDirValue = horPrewittArr[i*widthNoPad+j];
      yDirValue = verPrewittArr[i*widthNoPad+j];
      filteredIntegers[i*widthNoPad + j] = sqrt(xDirValue*xDirValue+yDirValue*yDirValue);
    }
  }
  free(verPrewittArr);
  free(horPrewittArr);
  pixelValues = filterSlice(filteredIntegers, width-padding*2, height-padding*2);
  free(filteredIntegers);
  pixelValues = addStaticPad(pixelValues, 0, width-padding*2, height-padding*2, kernelSize);
  writeArrToPgm(pixelValues, width, height, outputFileName, 2);
  free(pixelValues);
  return 0;
}

int applyVerPrewittPgm(char* inputFileName, char* outputFileName) {
  uint8_t *arr, *pixelValues;
  int width, height, *filteredIntegers, padding, kernelSize = 3;
  arr = rBinaryPgmPad(inputFileName, &width, &height, 0);
  if (!arr) {
    setColor(YELLOW);
    printf("Trying ASCII pgm format...\n");
    setColor(RESET);
    arr = rAsciiPgmPad(inputFileName, &width, &height, 0);
  }
  if (!arr)
    return 1;
  filteredIntegers = applyPrewittVertical(arr, width, height, true);
  free(arr);
  padding = (kernelSize>>1);
  pixelValues = filterSlice(filteredIntegers, width-padding*2, height-padding*2);
  free(filteredIntegers);
  pixelValues = addStaticPad(pixelValues, 0, width-padding*2, height-padding*2, kernelSize);
  writeArrToPgm(pixelValues, width, height, outputFileName, 2);
  free(pixelValues);
  return 0;
}

/*
* Function: applyPrewittVertical
* --------------------------
* returns a pointer to an array with vertical edge detection filter applied
*
* pixelValues: pointer to the array representing image that will be processed
* width: image width without padding
* height: image height without padding
* postPadding: a bool value indicating whether the padding will be applied
* before (false) or after (true) kernel
*
* returns: a pointer to the newly allocated array after processing
*/
int* applyPrewittVertical(uint8_t* pixelValues, int width, int height, bool postPadding) {
  int staticKernel[3*3] = {1, 1, 1, 0, 0, 0, -1, -1, -1};
  int kernelSize = 3, *kernel, padding, *filteredValues;
  uint8_t *pixelValuesCopy;
  if (!doesKernelFit(kernelSize, width, height) && !postPadding) {
    setColor(RED);
    printf("Kernel too be big to be applied without pre-padding\n");
    setColor(RESET);
    return NULL;
  }
  kernel = staticToDynamicKernel(staticKernel, kernelSize);
  pixelValuesCopy = (uint8_t*) malloc(width * height * sizeof(uint8_t));
  memcpy(pixelValuesCopy, pixelValues, width*height * sizeof(uint8_t));
  if (!postPadding) {
    pixelValuesCopy = addStaticPad(pixelValuesCopy, 0, width, height, kernelSize);
    padding = (kernelSize>>1);
  } else {
    padding = 0;
  }
  filteredValues = applyKernelArr(kernel, kernelSize, 1, pixelValuesCopy, width+padding*2, height+padding*2);
  free(pixelValuesCopy);
  free(kernel);
  return filteredValues;
}

/*
* Function: applyPrewittHorizontal
* --------------------------
* returns a pointer to an array with horizontal edge detection filter applied
*
* pixelValues: pointer to the array representing image that will be processed
* width: image width without padding
* height: image height without padding
* postPadding: a bool value indicating whether the padding will be applied
* before (false) or after (true) kernel
*
* returns: a pointer to the newly allocated array after processing
*/
int* applyPrewittHorizontal(uint8_t* pixelValues, int width, int height, bool postPadding) {
  int staticKernel[3*3] = {1, 0, -1, 1, 0, -1, 1, 0, -1};
  int kernelSize = 3, *kernel, padding, *filteredValues;
  uint8_t *pixelValuesCopy;
  if (!doesKernelFit(kernelSize, width, height) && !postPadding) {
    setColor(RED);
    printf("Kernel too be big to be applied without pre-padding\n");
    setColor(RESET);
    return NULL;
  }
  kernel = staticToDynamicKernel(staticKernel, kernelSize);
  pixelValuesCopy = (uint8_t*) malloc(width * height * sizeof(uint8_t));
  memcpy(pixelValuesCopy, pixelValues, width*height * sizeof(uint8_t));
  if (!postPadding) {
    pixelValuesCopy = addStaticPad(pixelValuesCopy, 0, width, height, kernelSize);
    padding = (kernelSize>>1);
  } else {
    padding = 0;
  }
  filteredValues = applyKernelArr(kernel, kernelSize, 1, pixelValuesCopy, width+padding*2, height+padding*2);
  free(pixelValuesCopy);
  free(kernel);
  return filteredValues;
}

uint8_t* rBinaryPgmPad(char* fileName, int* width, int* height, int kernelSize) {
  FILE* picture;
  uint8_t* pixelValues;
  int numberHolder;
  if (kernelSize < MIN_KERNEL_SIZE && kernelSize % 2) {
    setColor(RED);
    printf("Error: kernel size must be >= %d and odd", MIN_KERNEL_SIZE);
    setColor(RESET);
    return NULL;
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
  fscanf(picture, "%d %d", width, height);
  // get maximum value
  skipComments(picture);
  fscanf(picture, "%d", &numberHolder);
  fgetc(picture);
  // rowWidth is the width plus the padding on both sides
  pixelValues = (uint8_t*) malloc((*height) * (*width) * sizeof(uint8_t));
  if (!pixelValues) {
    setColor(RED);
    printf("%s:%d > Failed to allocate memory\n", __FILE__, __LINE__);
    setColor(RESET);
    return NULL;
  }
  fread(pixelValues, sizeof(uint8_t), (*width) * (*height), picture);
  fclose(picture);
  if (kernelSize) {
    return addStaticPad(pixelValues, 0, *width, *height, kernelSize);
  } else {
    return pixelValues;
  }
}

uint8_t* rAsciiPgmPad(char* fileName, int* width, int* height, int kernelSize) {
  FILE* picture;
  uint8_t* pixelValues;
  size_t i = 0;
  int numberHolder;
  if (kernelSize < MIN_KERNEL_SIZE && kernelSize % 2) {
    setColor(RED);
    printf("Error: kernel size must be >= %d and odd", MIN_KERNEL_SIZE);
    setColor(RESET);
    return NULL;
  }
  if (!(picture = fopen(fileName, "rb"))) {
    setColor(RED);
    printf("Couldn't read %s\nProbably doesn't exist\n", fileName);
    setColor(RESET);
    return NULL;
  }
  if (!isAsciiPgm(picture)) {
    setColor(RED);
    printf("%s is not an ASCII PGM file\n", fileName);
    setColor(RESET);
    fclose(picture);
    return NULL;
  }
  skipComments(picture);
  fscanf(picture, "%d %d", width, height);
  // get maximum value
  skipComments(picture);
  fscanf(picture, "%d", &numberHolder);
  fgetc(picture);
  pixelValues = (uint8_t*) malloc((*height) * (*width) * sizeof(uint8_t));
  if (!pixelValues) {
    setColor(RED);
    printf("%s:%d > Failed to allocate memory\n", __FILE__, __LINE__);
    setColor(RESET);
    return NULL;
  }
  while (fscanf(picture, " %d ", &numberHolder) == 1) {
    if (numberHolder > 255 || numberHolder < 0) {
      setColor(RED);
      printf("Error: corrupt input. Some pixels are not in [0-255]\n");
      setColor(RESET);
      fclose(picture);
      return NULL;
    }
    pixelValues[i++] = numberHolder;
  };
  if (i != (*height) * (*width)) {
    setColor(RED);
    printf("Error: corrupt input. Read %ld pixels, expected %d.\n", i, (*height) * (*width));
    setColor(RESET);
    fclose(picture);
    return NULL;
  }
  fclose(picture);
  if (kernelSize) {
    return addStaticPad(pixelValues, 0, *width, *height, kernelSize);
  } else {
    return pixelValues;
  }
}

uint8_t* allocArrWithPad(int width, int height, int kernelSize) {
  // divide by 2 (floor)
  int padding = kernelSize>>1;
  return (uint8_t*) malloc((width+padding*2)*(height+padding*2));
}

/*
* Function: addStaticPad
* --------------------------
* returns a pointer to an array with border padding set to paddingVal
* and frees the pointer specified by pixelValues
*
* pixelValues: pointer to the array that will be padded
* paddingVal: the value that padded pixels will be set to
* width: image width without padding
* height: image height without padding
* kernelSize: the length of kernel dimension
*
* returns: a pointer to the newly allocated array with padding
*/
uint8_t* addStaticPad(uint8_t* pixelValues, uint8_t paddingVal, int width, int height, int kernelSize) {
  size_t i, j;
  uint8_t* arrValues;
  int padding = kernelSize>>1;
  // rowWidth is the width plus the padding on both sides
  int rowWidth = width + padding*2;
  arrValues = allocArrWithPad(width, height, kernelSize);
  if (!arrValues) {
    setColor(RED);
    printf("%s:%d > Failed to allocate memory\n", __FILE__, __LINE__);
    setColor(RESET);
    return NULL;
  }
  //initialize padding rows with paddingVal
  for (j=0, i=0; j < padding; i++, j++)
    memset(arrValues + i*rowWidth, paddingVal, rowWidth);
  //set pixelvalues adding padding at the beginning and end of each row
  for (j=0; j < height; i++, j++) {
    memset(arrValues + i*rowWidth, paddingVal, padding);
    memcpy(arrValues + i*rowWidth+ padding, pixelValues + j*width, width * sizeof(uint8_t));
    memset(arrValues + i*rowWidth + padding + width, paddingVal, padding);
  }
  //initialize last padding rows with paddingVal
  for (j=0; j < padding; i++,j++)
    memset(arrValues + i*rowWidth, paddingVal, rowWidth);
  free(pixelValues);
  return arrValues;
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
  int (*applyKernelPix)(int*, int, float, uint8_t*, int, size_t, size_t) = applyKernelPixWithCo;
  if (coefficient == 1)
    applyKernelPix = applyKernelPixNoCo;
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

int applyKernelPixWithCo(int* kernel, int kernelSize, float coefficient, uint8_t* pixelValues, int width, size_t xLoc, size_t yLoc) {
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

int applyKernelPixNoCo(int* kernel, int kernelSize, float coefficient, uint8_t* pixelValues, int width, size_t xLoc, size_t yLoc) {
  size_t i, j;
  int result = 0, kernelPadding, row;
  kernelPadding = kernelSize >> 1;
  for (i = 0; i < kernelSize; i++) {
    row = xLoc+i-kernelPadding;
    for (j = 0; j < kernelSize; j++){
      result += pixelValues[(yLoc+j-kernelPadding)+row*width] * kernel[i*kernelSize+j];
    }
  }
  return result;
}

/*
* Function: filterSlice
* --------------------------
* sets all values grater than 255 to 255 and all values less than 0 to 0
*
* arrValues: a pointer the array that should be filtered
* width: array width
* height: array height
*
* returns: a pointer to the allocated array with filtered values
*/
uint8_t* filterSlice(int* arrValues, int width, int height) {
  size_t i;
  uint8_t* outputPixValues = (uint8_t*) malloc(width * height * sizeof(uint8_t));
  for (i = 0; i < height*width; i++) {
    outputPixValues[i] = (uint8_t)(arrValues[i] < 0 ? 0 : (arrValues[i] > MAX_PIXEL_VAL ? MAX_PIXEL_VAL : arrValues[i]));
  }
  return outputPixValues;
}

/*
* Function: filterMinMax
* --------------------------
* normalizes arrValues values between [0-255]
*
* arrValues: a pointer the array that should be normalized
* width: array width
* height: array height
*
* returns: a pointer to the allocated array with normalized values
*/
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
  if (srcMax == srcMin) {
    if (srcMin <= 255 && srcMin >= 0) {
      for (i = 0; i < width*height; i++)
        outputPixValues[i] = arrValues[i];
    } else if (srcMin >= 255) {
      memset(outputPixValues, 255, width*height);
    } else {
      memset(outputPixValues, 0, width*height);
    }
    return outputPixValues;
  } else if (srcMin <= 255 && srcMax >= 0) {
    for (i = 0; i < width*height; i++)
      outputPixValues[i] = arrValues[i];
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

/*
* Function: setPaddingMirror
* --------------------------
* modifies arr by setting the values of the added padding pixels
* to the nearest neighbor values
*
* arr: pixel values array with padding allocated
* width: image width without padding
* height: image height without padding
* kernelSize: the length of kernel dimension
*
* returns: a bool value indicating failure as false and success as true
*/
void setPaddingMirror(uint8_t* arr, int width, int height, int kernelSize) {
  size_t i, j, currPixLoc, rowWidth;
  int padding = kernelSize >> 1;
  rowWidth = padding*2+width;
  // set top-left padding
  currPixLoc = (padding*2+width)*padding+padding;
  for (i = 0; i < padding; i++) {
    for (j = 0; j < padding; j++)
      arr[i*(padding*2+width)+j] = arr[currPixLoc];
  }
  // set top-right padding
  currPixLoc = (padding*2+width)*padding+padding+width-1;
  for (i = 0; i < padding; i++) {
    for (j = 0; j < padding; j++)
      arr[i*rowWidth+j+padding+width] = arr[currPixLoc];
  }
  // set bottom-left padding
  currPixLoc = (padding*2+width)*(padding+height-1)+padding;
  for (i = 0; i < padding; i++) {
    for (j = 0; j < padding; j++)
      arr[(i+height+padding)*rowWidth+j] = arr[currPixLoc];
  }
  // set bottom-right padding
  currPixLoc = (padding*2+width)*(padding+height-1)+padding+width-1;
  for (i = 0; i < padding; i++) {
    for (j = 0; j < padding; j++)
      arr[(i+padding+height)*rowWidth+width+padding+j] = arr[currPixLoc];
  }
  // set top border
  for (i = 0; i < padding; i++) {
    for (j = 0; j < width; j++)
      arr[padding + i*rowWidth +j] = arr[padding + rowWidth*padding + j];
  }
  //set bottom border
  for (i = 0; i < padding; i++) {
    for (j = 0; j < width; j++)
      arr[padding + (padding+height+i)*rowWidth +j] = arr[padding + rowWidth*(padding+height-1) + j];
  }
  // set left border
  for (i = 0; i < height; i++) {
    for (j = 0; j < padding; j++)
      arr[rowWidth*(i+padding)+j] = arr[rowWidth*(i+padding)+padding];
  }
  // set right border
  for (i = 0; i < height; i++) {
    for (j = 0; j < padding; j++)
      arr[rowWidth*(i+padding)+j+padding+width] = arr[rowWidth*(i+padding)+padding+width-1];
  }
}

bool isBinaryPgm(FILE* file) {
  char* typeHolder = (char*) malloc(LINE_SIZE * sizeof(char));
  fread(typeHolder, sizeof(char), MAGIC_NUMBER_SIZE, file);
  typeHolder[MAGIC_NUMBER_SIZE] = '\0';
  strtok(typeHolder, "\n");
  if (strcmp(typeHolder, "P5")) {
    free(typeHolder);
    return false;
  }
  free(typeHolder);
  return true;
}

bool isAsciiPgm(FILE* file) {
  char* typeHolder = (char*) malloc(LINE_SIZE * sizeof(char));
  fread(typeHolder, sizeof(char), MAGIC_NUMBER_SIZE, file);
  typeHolder[MAGIC_NUMBER_SIZE] = '\0';
  strtok(typeHolder, "\n");
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
  fprintf(picture, "# processed by %s\n", BRAND_NAME);
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

/*
* Function: staticToDynamic
* --------------------------
* returns a pointer to a user allocated array containing the same values of arr
*
* arr: pointer to the array that will be copied
* kernelSize: the length of kernel dimension
*
* returns: a pointer to the allocated array with copied values
*/
int* staticToDynamicKernel(int arr[], int kernelSize) {
  int* arrVal = malloc(kernelSize * kernelSize * sizeof(int));
  memcpy(arrVal, arr, kernelSize * kernelSize * sizeof(int));
  return arrVal;
}

/*
* Function: doesKernelFit
* --------------------------
* checks if the kernel size fits the image without padding
*
* width: image width without padding
* height: image height without padding
* kernelSize: the length of kernel dimension
*
* returns: a bool indicating if the kernel can be applied before padding
*/
bool doesKernelFit(int kernelSize, int width, int height) {
  return width > kernelSize && height > kernelSize;
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
