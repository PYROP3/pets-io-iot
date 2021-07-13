#ifndef QR_H

#define QR_H

//typedef struct {
//    uint8_t * buf;              /*!< Pointer to the pixel data */
//    size_t len;                 /*!< Length of the buffer in bytes */
//    size_t width;               /*!< Width of the buffer in pixels */
//    size_t height;              /*!< Height of the buffer in pixels */
//    pixformat_t format;         /*!< Format of the pixel data */
//    struct timeval timestamp;   /*!< Timestamp since boot of the first DMA buffer of the frame */
//} camera_fb_t;

#define IS_BLACK(pix) pix < 128
#define IS_WHITE(pix) pix >= 128

#define _VX(vec) vec[0]
#define _VY(vec) vec[1]
#define _SZ(vec) vec[2]
#define _VEC2(vec) _VX(vec), _VY(vec)

#define SQ(x) ((float)x)*((float)x)

#define MASK_ODD_STATE 0x1

#define MAX_FINDERS 5

// TODO calculate automatically?
#define QR_N 29
#define QR_TIMING_START 6

int finders[MAX_FINDERS][3]; // -> [x][y][size]
int total_finders = 0;
int anchor[2];

float perspective_transform[3][3] = {0.};
int transformed_coords[2];

bool rawCode[QR_N][QR_N] = {false};

typedef struct {
  String registerToken;
  String ap_ssid;
  String ap_password;
} parsed_json_t;

int findCenter(int *ratio, int ratioEnd) {
  return ratioEnd - ratio[4] - ratio[3] - (ratio[2]/2);
}

void printMat3x4(float mat[3][4]) {
  Serial.printf("scanQR: [%.3f, %.3f, %.3f | %.3f]\n", mat[0][0], mat[0][1], mat[0][2], mat[0][3]);
  Serial.printf("scanQR: [%.3f, %.3f, %.3f | %.3f]\n", mat[1][0], mat[1][1], mat[1][2], mat[1][3]);
  Serial.printf("scanQR: [%.3f, %.3f, %.3f | %.3f]\n", mat[2][0], mat[2][1], mat[2][2], mat[2][3]);
  Serial.println("");
}

float * gaussReduce(float mat[3][4]) {
  int i;
  static float result[3];
  float k;

  Serial.println("start gaussReduce ===============");

  printMat3x4(mat);
  //( p1x p2x p0x | ax )
  //( p1y p2y p0y | ay )
  //( 1   1   1   | 1  )

  // Divide first line by first element
  k = mat[0][0];
  Serial.printf("scanQR: Divide first k=%.3f\n", k);
  for (i=0;i<4;i++) {
    mat[0][i] = mat[0][i] / k;
  }

  printMat3x4(mat);
  //( 1   ?   ?   | ?  )
  //( p1y p2y p0y | ay )
  //( 1   1   1   | 1  )

  // Reduce second and third lines
  k = mat[1][0];
  Serial.printf("scanQR: Reduce second & third k=%.3f\n", k);
  for (i=0;i<4;i++) {
    mat[1][i] = mat[1][i] - (mat[0][i] * k);
  }

  printMat3x4(mat);
  //( 1 ? ? | ? )
  //( 0 ? ? | ? )
  //( 1 1 1 | 1 )
  
  k = mat[2][0];
  Serial.printf("scanQR: Reduce second & third k=%.3f\n", k);
  for (i=0;i<4;i++) {
    mat[2][i] = mat[2][i] - (mat[0][i] * k);
  }

  printMat3x4(mat);
  //( 1 ? ? | ? )
  //( 0 ? ? | ? )
  //( 0 ? ? | ? )

  // Divide second line by second element
  k = mat[1][1];
  Serial.printf("scanQR: Divide second k=%.3f\n", k);
  for (i=1;i<4;i++) {
    mat[1][i] = mat[1][i] / k;
  }

  printMat3x4(mat);
  //( 1 ? ? | ? )
  //( 0 1 ? | ? )
  //( 0 ? ? | ? )

  // Reduce third line
  k = mat[2][1];
  Serial.printf("scanQR: Reduce third k=%.3f\n", k);
  for (i=0;i<4;i++) {
    mat[2][i] = mat[2][i] - (mat[1][i] * k);
  }

  printMat3x4(mat);
  //( 1 ? ? | ? )
  //( 0 1 ? | ? )
  //( 0 0 ? | ? )

  // Divide third line by third element
  k = mat[2][2];
  Serial.printf("scanQR: Divide third k=%.3f\n", k);
  for (i=2;i<4;i++) {
    mat[2][i] = mat[2][i] / k;
  }

  printMat3x4(mat);
  //( 1 ? ? | ? )
  //( 0 1 ? | ? )
  //( 0 0 1 | ? )

  // Reduce first and second lines
  k = mat[0][2];
  Serial.printf("scanQR: Reduce first k=%.3f\n", k);
  for (i=2;i<4;i++) {
    mat[0][i] = mat[0][i] - (mat[2][i] * k);
  }

  printMat3x4(mat);
  //( 1 ? 0 | ? )
  //( 0 1 ? | ? )
  //( 0 0 1 | ? )
  
  k = mat[1][2];
  Serial.printf("scanQR: Reduce second k=%.3f\n", k);
  for (i=1;i<4;i++) {
    mat[1][i] = mat[1][i] - (mat[2][i] * k);
  }

  printMat3x4(mat);
  //( 1 ? 0 | ? )
  //( 0 1 0 | ? )
  //( 0 0 1 | ? )

  // Reduce first line again
  k = mat[0][1];
  Serial.printf("scanQR: Reduce first k=%.3f\n", k);
  for (i=0;i<4;i++) {
    mat[0][i] = mat[0][i] - (mat[1][i] * k);
  }

  printMat3x4(mat);
  //( 1 0 0 | ? )
  //( 0 1 0 | ? )
  //( 0 0 1 | ? )

  // Copy results and return
  for (i=0;i<3;i++) {
    result[i] = mat[i][3];
  }
  
  Serial.println("finish gaussReduce ===============");
  
  return result;
}

boolean checkRatio(int *ratio) {
  int total_size = 0;
  int ratio_ptr = 0;
  int count;
  int module_size;
  int max_var;
  
  // Serial.printf("scanQR: checkRatio: [%d,%d,%d,%d,%d]\n", ratio[0], ratio[1], ratio[2], ratio[3], ratio[4]);
  
  for (ratio_ptr = 0; ratio_ptr < 5; ratio_ptr++) {
    count = ratio[ratio_ptr];
    if (count == 0) return false;
    total_size += count;
  }
  if (total_size < 7) { // 1 + 1 + 3 + 1 + 1
    return false;
  }

  module_size = ceil(total_size / 7.0);
  max_var = module_size / 2;

  return ((abs(module_size - ratio[0]) < max_var) &&
      (abs(module_size - ratio[1]) < max_var) &&
      (abs(3*module_size - ratio[2]) < 3*max_var) &&
      (abs(module_size - ratio[3]) < max_var) &&
      (abs(module_size - ratio[4]) < max_var));
}

int distSquared(int x1, int y1, int x2, int y2) {
  return ((x1 - x2) * (x1 - x2)) + ((y1 - y2) * (y1 - y2));
}

int checkVertical(uint8_t * buf, int y, int x, int ratioMiddle, int ratioTotal, int max_x, int max_y) {
  int checkStates[5] = {0};
  int checkTotal;
  int i;
  int curr_y = y;
  
  // Move up from center and check for validity
  while (curr_y >= 0 && IS_BLACK(buf[x + curr_y * max_x])) {
    checkStates[2]++;
    curr_y--;
  }
  
  if (curr_y<0) { // Not a center (out of image)
#ifdef DEBUG
    Serial.printf("scanQR: checkVertical negative Y\n");
#endif
    return -1;
  }

  while (curr_y >= 0 && IS_WHITE(buf[x + curr_y * max_x]) && checkStates[1] < ratioMiddle) {
    checkStates[1]++;
    curr_y--;
  }
  
  if (curr_y<0 || checkStates[1]>ratioMiddle) { // Not a center (out of image or center is smaller than rest)
#ifdef DEBUG
    Serial.printf("scanQR: checkVertical negative Y (%d) or ratio error (%d/%d)\n", curr_y, checkStates[1], ratioMiddle);
    Serial.printf("scanQR: checkVertical: [%d,%d,%d,%d,%d]\n", checkStates[0], checkStates[1], checkStates[2], checkStates[3], checkStates[4]);
#endif
    return -1;
  }

  while (curr_y >= 0 && IS_BLACK(buf[x + curr_y * max_x]) && checkStates[0] < ratioMiddle) {
    checkStates[0]++;
    curr_y--;
  }
  
  if (curr_y<0 || checkStates[0]>ratioMiddle) { // Not a center (out of image or center is smaller than rest)
#ifdef DEBUG
    Serial.printf("scanQR: checkVertical negative Y (%d) or ratio error (%d/%d)\n", curr_y, checkStates[0], ratioMiddle);
    Serial.printf("scanQR: checkVertical: [%d,%d,%d,%d,%d]\n", checkStates[0], checkStates[1], checkStates[2], checkStates[3], checkStates[4]);
#endif
    return -1;
  }

  // Move down from center and check for validity
  curr_y = y + 1;
  
  while (curr_y < max_y && IS_BLACK(buf[x + curr_y * max_x])) {
    checkStates[2]++;
    curr_y++;
  }
  
  if (curr_y == max_y) { // Not a center (out of image)
#ifdef DEBUG
    Serial.printf("scanQR: checkVertical out of bounds Y\n");
#endif
    return -1;
  }
  
  while (curr_y < max_y && IS_WHITE(buf[x + curr_y * max_x]) && checkStates[3] < ratioMiddle) {
    checkStates[3]++;
    curr_y++;
  }
  
  if (curr_y == max_y || checkStates[3]>ratioMiddle) { // Not a center (out of image or center is smaller than rest)
#ifdef DEBUG
    Serial.printf("scanQR: checkVertical out of bounds Y (%d) or ratio error (%d/%d)\n", curr_y, checkStates[3], ratioMiddle);
    Serial.printf("scanQR: checkVertical: [%d,%d,%d,%d,%d]\n", checkStates[0], checkStates[1], checkStates[2], checkStates[3], checkStates[4]);
#endif
    return -1;
  }

  while (curr_y < max_y && IS_BLACK(buf[x + curr_y * max_x]) && checkStates[4] < ratioMiddle) {
    checkStates[4]++;
    curr_y++;
  }
  
  if (curr_y == max_y || checkStates[4]>ratioMiddle) { // Not a center (out of image or center is smaller than rest)
#ifdef DEBUG
    Serial.printf("scanQR: checkVertical out of bounds Y (%d) or ratio error (%d/%d)\n", curr_y, checkStates[4], ratioMiddle);
    Serial.printf("scanQR: checkVertical: [%d,%d,%d,%d,%d]\n", checkStates[0], checkStates[1], checkStates[2], checkStates[3], checkStates[4]);
#endif
    return -1;
  }

  // Check the ratio again
  checkTotal = 0;
  for (i = 0; i < 5; i++) {
    checkTotal += checkStates[i];
  }

  if (5*abs(checkTotal-ratioTotal) >= 2*ratioTotal) {
#ifdef DEBUG
    Serial.printf("scanQR: checkVertical  ratio error (%d/%d)\n", 5*abs(checkTotal-ratioTotal), 2*ratioTotal);
#endif
    return -1;
  }

#ifdef DEBUG
  Serial.printf("scanQR: checkVertical -> checkRatio: [%d,%d,%d,%d,%d]\n", checkStates[0], checkStates[1], checkStates[2], checkStates[3], checkStates[4]);
#endif

  int center = findCenter(checkStates, curr_y);
  return checkRatio(checkStates) ? center : -1;
}

int checkHorizontal(uint8_t * buf, int y, int x, int ratioMiddle, int ratioTotal, int max_x, int max_y) {
  int checkStates[5] = {0};
  int checkTotal;
  int i;
  int curr_x = x;
  
  // Move right from center and check for validity
  while (curr_x >= 0 && IS_BLACK(buf[curr_x + y * max_x])) {
    checkStates[2]++;
    curr_x--;
  }
  
  if (curr_x<0) { // Not a center (out of image)
    return -1;
  }

  while (curr_x >= 0 && IS_WHITE(buf[curr_x + y * max_x]) && checkStates[1] < ratioMiddle) {
    checkStates[1]++;
    curr_x--;
  }
  
  if (curr_x<0 || checkStates[1]>=ratioMiddle) { // Not a center (out of image or center is smaller than rest)
    return -1;
  }

  while (curr_x >= 0 && IS_BLACK(buf[curr_x + y * max_x]) && checkStates[0] < ratioMiddle) {
    checkStates[0]++;
    curr_x--;
  }
  
  if (curr_x<0 || checkStates[0]>=ratioMiddle) { // Not a center (out of image or center is smaller than rest)
    return -1;
  }

  // Move left from center and check for validity
  curr_x = x + 1;
  
  while (curr_x < max_x && IS_BLACK(buf[curr_x + y * max_x])) {
    checkStates[2]++;
    curr_x++;
  }
  
  if (curr_x == max_x) { // Not a center (out of image)
    return -1;
  }
  
  while (curr_x < max_x && IS_WHITE(buf[curr_x + y * max_x]) && checkStates[3] < ratioMiddle) {
    checkStates[3]++;
    curr_x++;
  }
  
  if (curr_x == max_x || checkStates[3]>=ratioMiddle) { // Not a center (out of image or center is smaller than rest)
    return -1;
  }

  while (curr_x < max_x && IS_BLACK(buf[curr_x + y * max_x]) && checkStates[4] < ratioMiddle) {
    checkStates[4]++;
    curr_x++;
  }
  
  if (curr_x == max_x || checkStates[4]>=ratioMiddle) { // Not a center (out of image or center is smaller than rest)
    return -1;
  }

  // Check the ratio again
  checkTotal = 0;
  for (i = 0; i < 5; i++) {
    checkTotal += checkStates[i];
  }

  if (5*abs(checkTotal-ratioTotal) >= 2*ratioTotal) {
    return -1;
  }

  int center = findCenter(checkStates, curr_x);
  return checkRatio(checkStates) ? center : -1;
}

bool checkDiagonal(uint8_t * buf, int y, int x, int ratioMax, int ratioTotal, int max_x, int max_y) {
  int checkStates[5] = {0};
  int checkTotal;
  int i;

  // Move from center to top-left, staying in bounds
  i = 0;
  while (y >= i && x>=i && IS_BLACK(buf[x-i + (y-i) * max_x])) {
    checkStates[2]++;
    i++;
  }

  if (y<i || x<i) return false;
  
  while (y >= i && x >= i && IS_WHITE(buf[x-i + (y-i) * max_x]) && checkStates[1]<=ratioMax) {
    checkStates[1]++;
    i++;
  }

  if (y<i || x<i || checkStates[1]>ratioMax) return false;
  
  while (y >= i && x >= i && IS_BLACK(buf[x-i + (y-i) * max_x]) && checkStates[0]<=ratioMax) {
    checkStates[0]++;
    i++;
  }

  if (checkStates[0]>ratioMax) return false;

  // Move from center to bottom-right, staying in bounds
  i = 1;
  while ((y+i)<max_y && (x+i)<max_x && IS_BLACK(buf[x+i + (y+i) * max_x]) && checkStates[2]<=ratioMax) {
    checkStates[2]++;
    i++;
  }

  if ((y+i)>=max_y || (x+i)>=max_x) return false;
  
  while ((y+i)<max_y && (x+i)<max_x && IS_WHITE(buf[x+i + (y+i) * max_x]) && checkStates[3]<=ratioMax) {
    checkStates[3]++;
    i++;
  }

  if ((y+i)>=max_y || (x+i)>=max_x || checkStates[3]>ratioMax) return false;
  
  while ((y+i)<max_y && (x+i)<max_x && IS_BLACK(buf[x+i + (y+i) * max_x]) && checkStates[4]<=ratioMax) {
    checkStates[4]++;
    i++;
  }

  if ((y+i)>=max_y || (x+i)>=max_x || checkStates[4]>ratioMax) return false;

  // Check the ratio one last time
  checkTotal = 0;
  for (i = 0; i < 5; i++) {
    checkTotal += checkStates[i];
  }

  return (abs(ratioTotal - checkTotal) < 2*ratioTotal) && checkRatio(checkStates);
}

bool handleFinder(uint8_t * buf, int * ratio, int x, int y, int max_x, int max_y) {
  int total_size = 0;
  int ratio_ptr;
  int center_x, center_y;
  bool validFinder;
  bool found;
  int finder_idx;
  // int estimated_size = total_size / 7;
  
#ifdef DEBUG
  Serial.printf("scanQR: handleFinder ratio: [%d,%d,%d,%d,%d]\n", ratio[0], ratio[1], ratio[2], ratio[3], ratio[4]);
#endif

  // Out of memory
  if (total_finders >= MAX_FINDERS) return false;
#ifdef DEBUG
  Serial.printf("scanQR: MAX ok\n");
#endif
  
  for (ratio_ptr = 0; ratio_ptr < 5; ratio_ptr++) {
    total_size += ratio[ratio_ptr];
  }

  center_x = findCenter(ratio, x);

  // Check along the vertical
#ifdef DEBUG
  Serial.printf("scanQR: Checking vertical\n");
#endif
  center_y = checkVertical(buf, y, center_x, ratio[2], total_size, max_x, max_y);
  if (center_y < 0) return false;

  // Check along the horizontal again
#ifdef DEBUG
  Serial.printf("scanQR: Checking horizontal\n");
#endif
  center_x = checkHorizontal(buf, center_y, center_x, ratio[2], total_size, max_x, max_y);
  if (center_x < 0) return false;

  // Check along the diagonal from centers
#ifdef DEBUG
  Serial.printf("scanQR: Checking diagonal\n");
#endif
  validFinder = checkDiagonal(buf, center_y, center_x, ratio[2], total_size, max_x, max_y);
  if (!validFinder) return false;

#ifdef DEBUG
  Serial.printf("scanQR: Checking duplicates\n");
#endif
//  estimated_size = total_size/7;
  found = false;
  for (finder_idx = 0; finder_idx < total_finders; finder_idx++) {
    if (distSquared(finders[finder_idx][0], finders[finder_idx][1], center_x, center_y) < 100) { // Too close, same finder
      finders[finder_idx][0] = (finders[finder_idx][0] + center_x) / 2;
      finders[finder_idx][1] = (finders[finder_idx][1] + center_y) / 2;
      finders[finder_idx][2] = (finders[finder_idx][2] + ratio[2]) / 2;
#ifdef DEBUG
      Serial.printf("scanQR: Found duplicated finder [%d], updating (%d, %d, %d) => (%d, %d, %d)\n", finder_idx, center_x, center_y, estimated_size, finders[finder_idx][0],finders[finder_idx][1], finders[finder_idx][2]);
#endif
      found = true;
      break;
    }
  }

  if (!found) { // Found new finder!
    finders[total_finders][0] = center_x;
    finders[total_finders][1] = center_y;
    finders[total_finders][2] = ratio[2];
    total_finders++;
#ifdef DEBUG
    Serial.printf("Found new finder!\n");
#endif
  }

  return true;
}

boolean findFinders(uint8_t *buf, int width, int height) {
  int x, y;
  int ratio[5];
  int ratio_ptr = 0;
#ifdef DEBUG
  boolean confirmed;
#endif
 
  for (y=0;y<height;y++) {
    memset(ratio, 0, 5*sizeof(int));
    ratio_ptr = 0;
    for (x=0;x<width;x++) {
      if (IS_BLACK(buf[x + y * width])) { // If in black
        if (ratio_ptr & MASK_ODD_STATE) { // Odd states are for white -> move
          ratio_ptr++;
        }
        ratio[ratio_ptr]++;
      } else { // If in white
        if (ratio_ptr & MASK_ODD_STATE) { // Odd states are for white -> increase
          ratio[ratio_ptr]++;
        } else { // Check for end of finder
          if (ratio_ptr == 4) { // Found all ranges, check
            if (checkRatio(ratio)) { // Found a finder
#ifdef DEBUG
              confirmed = handleFinder(buf, ratio, x, y, width, height);
              if (confirmed) {
                Serial.printf("scanQR: CONFIRMED finder @ (%d,%d)\n", x, y);
              } else {
                Serial.printf("scanQR: false positive @ (%d,%d)\n", x, y);
              }
#else
              handleFinder(buf, ratio, x, y, width, height);
#endif
            } else { // Roll back and continue
              ratio[0] = ratio[2];
              ratio[1] = ratio[3];
              ratio[2] = ratio[4];
              ratio[3] = 1;
              ratio[4] = 0;
              ratio_ptr = 3;
              continue;
            }
            memset(ratio, 0, 5*sizeof(int));
            ratio_ptr = 0;
          } else { // Just another transition
            ratio_ptr++;
            ratio[ratio_ptr]++;
          }
        }
      }
    }
  }

  if (total_finders == 3) {
    Serial.printf("scanQR: found %d finders\n", total_finders);
    for (ratio_ptr = 0; ratio_ptr < total_finders; ratio_ptr++) {
      Serial.printf("scanQR:\t(%d, %d, %d)\n", finders[ratio_ptr][0], finders[ratio_ptr][1], finders[ratio_ptr][2]);
    }
  }

#ifdef DEBUG
  Serial.printf("scanQR: finished!\n");
#endif

  return total_finders == 3;
}

void reorderFinders() {
  // Set finders[0] to corner
  int d01 = distSquared(_VX(finders[0]), _VY(finders[0]), _VX(finders[1]), _VY(finders[1]));
  int d12 = distSquared(_VX(finders[2]), _VY(finders[2]), _VX(finders[1]), _VY(finders[1]));
  int d02 = distSquared(_VX(finders[0]), _VY(finders[0]), _VX(finders[2]), _VY(finders[2]));
  Serial.printf("scanQR: reorderFinders d01=%d, d12=%d, d02=%d\n", d01, d12, d02);
  int temp[3];
  int swap = 0;

  if (d01 > d02) { // 0 or 2 is corner
    if (d01 > d12) { // 2 is corner
      swap = 2;
    } else { // 0 is already corner
      
    }
  } else { // 0 or 1 is corner
    if (d02 > d12) { // 1 is corner
      swap = 1;
    } else { // 0 is already corner
      
    }
  }
  Serial.printf("scanQR: reorderFinders : swap=%d\n", swap);

  if (swap != 0) {
      _VX(temp) = _VX(finders[0]);
      _VY(temp) = _VY(finders[0]);
      _SZ(temp) = _SZ(finders[0]);
      _VX(finders[0]) = _VX(finders[swap]);
      _VY(finders[0]) = _VY(finders[swap]);
      _SZ(finders[0]) = _SZ(finders[swap]);
      _VX(finders[swap]) = _VX(temp);
      _VY(finders[swap]) = _VY(temp);
      _SZ(finders[swap]) = _SZ(temp);
    for (swap = 0; swap < total_finders; swap++) {
      Serial.printf("scanQR: reordered finders\t(%d, %d, %d)\n", finders[swap][0], finders[swap][1], finders[swap][2]);
    }
    Serial.println("");
  }
}

float modCrossProduct(int *a, int *b) {
  return ((float)_VX(a) * (float)_VY(b)) - ((float)_VX(b) * (float)_VY(a));
}

void getAnchor() {
  int p12[2];
  int p13[2];
  int _ca[2];
  int _ab[2];
  int _cd[2];
  float k;

  Serial.printf("scanQR: getAnchor\n");

//  if (s1 == s2) // parallel
  if (_SZ(finders[0]) == _SZ(finders[1])) {
    Serial.printf("scanQR: 0-1 parallel\n");
//  endpoint x = x3 + (x1 - x2)
//  endpoint y = y3 + (y1 - y2)
//  line1_2 = (x3, y3) + t(x2 - x1, y2 - y1)
    _VX(p12) = _VX(finders[2]) + _VX(finders[0]) - _VX(finders[1]);
    _VY(p12) = _VY(finders[2]) + _VY(finders[0]) - _VY(finders[1]);
  } else {
//  else // perspective
//  // versor = (x1 - x2, y1 - y2) / (s2 - s1)
//  // endpoint = p1 + versor * s1
//
//  endpoint x = x1 + (x1 - x2) / (s2 - s1) * s1
//  endpoint y = y1 + (y1 - y2) / (s2 - s1) * s1
//
//  line1_2 = (x3, y3) + t(endpointx - x3, endpointy - y3)
    k = (float)_SZ(finders[0]) / ((float)_SZ(finders[1]) - (float)_SZ(finders[0]));
    Serial.printf("scanQR: 0-1 NOT parallel k=%.3f\n", k);
    _VX(p12) = _VX(finders[0]) + ((_VX(finders[0]) - _VX(finders[1])) * k);
    _VY(p12) = _VY(finders[0]) + ((_VY(finders[0]) - _VY(finders[1])) * k);
  }
  Serial.printf("scanQR: 0-1 fugue %d,%d\n", _VEC2(p12));
  
//  if (s1 == s3) // parallel
  if (_SZ(finders[0]) == _SZ(finders[2])) {
    Serial.printf("scanQR: 0-2 parallel\n");
//  endpoint x = x2 + (x1 - x3)
//  endpoint y = y2 + (y1 - y3)
//  line1_3 = (x2, y2) + t(x3 - x1, y3 - y1)
    _VX(p13) = _VX(finders[1]) + _VX(finders[0]) - _VX(finders[2]);
    _VY(p13) = _VY(finders[1]) + _VY(finders[0]) - _VY(finders[2]);
  } else {
//  else // perspective
//  // versor = (x1 - x3, y1 - y3) / (s3 - s1)
//  // endpoint = p1 + versor * s1
//
//  endpoint x = x1 + (x1 - x2) / (s3 - s1) * s1
//  endpoint y = y1 + (y1 - y2) / (s3 - s1) * s1
//
//  line1_3 = (x2, y2) + t(endpointx - x2, endpointy - y2)
    k = (float)_SZ(finders[0]) / ((float)_SZ(finders[2]) - (float)_SZ(finders[0]));
    Serial.printf("scanQR: 0-2 NOT parallel k=%.3f\n", k);
    _VX(p13) = _VX(finders[0]) + ((_VX(finders[0]) - _VX(finders[2])) * k);
    _VY(p13) = _VY(finders[0]) + ((_VY(finders[0]) - _VY(finders[2])) * k);
  }
  Serial.printf("scanQR: 0-2 fugue %d,%d\n", _VEC2(p13));

//            C e1_3
//                |
//                |
//            D  p2
//  A       B     |
//e1_2 ---- p3----X <- anchor
//CA × AB = s (CD × AB) => s = (CA × AB) / (CD × AB)
//P = C + s CD

  _VX(_ca) = _VX(p12) - _VX(p13);
  _VY(_ca) = _VY(p12) - _VY(p13);
  _VX(_ab) = _VX(finders[2]) - _VX(p12);
  _VY(_ab) = _VY(finders[2]) - _VY(p12);
  _VX(_cd) = _VX(finders[1]) - _VX(p13);
  _VY(_cd) = _VY(finders[1]) - _VY(p13);
  
  Serial.printf("scanQR: ca %d,%d\n", _VEC2(_ca));
  Serial.printf("scanQR: ab %d,%d\n", _VEC2(_ab));
  Serial.printf("scanQR: cd %d,%d\n", _VEC2(_cd));
  
  k = modCrossProduct(_ca, _ab) / modCrossProduct(_cd, _ab);
  Serial.printf("scanQR: k=%.3f\n", k);
  _VX(anchor) = _VX(p13) + k * _VX(_cd);
  _VY(anchor) = _VY(p13) + k * _VY(_cd);
  Serial.printf("scanQR: anchor @ %d,%d\n", _VEC2(anchor));
}

void applyTransform(int x, int y) {
  float _xyz[3];
  int i;

  for (i = 0; i < 3; i++) {
    _xyz[i] = perspective_transform[i][0] * x + perspective_transform[i][1] * y + perspective_transform[i][2];
  }
  
  _VX(transformed_coords) = ((int)(_xyz[0]/_xyz[2]));
  _VY(transformed_coords) = ((int)(_xyz[1]/_xyz[2]));

  Serial.printf("scanQR: applied transform %d,%d => %d,%d\n", x, y, _VEC2(transformed_coords));
}

void calculatePerspective() {
  float k = ((float)QR_N);
//  float k2_8k = SQ(k) - (8. * k);
//  float kp8 = k + 8.;
//  float k_82 = SQ(k - 8.);
  float invk_6 = 1. / (k - 6.);
  float ko6_k = k/(6.- k);

  int i, j, n;

//  float transformAPrime[3][3] = {{kp8/k2_8k,          0, -4*kp8/k2_8k},
//                                 {0,          kp8/k2_8k, -4*kp8/k2_8k},
//                                 {-kp8/k_82, -kp8/k_82 ,   k*kp8/k_82}};
  float transformAPrime[3][3] = {{     0, invk_6, -4*invk_6},
                                 {invk_6,      0, -4*invk_6},
                                 {invk_6, invk_6,     ko6_k}};

  Serial.printf("scanQR:      [%.3f, %.3f, %.3f]\n", transformAPrime[0][0], transformAPrime[0][1], transformAPrime[0][2]);
  Serial.printf("scanQR: A' = [%.3f, %.3f, %.3f]\n", transformAPrime[1][0], transformAPrime[1][1], transformAPrime[1][2]);
  Serial.printf("scanQR:      [%.3f, %.3f, %.3f]\n", transformAPrime[2][0], transformAPrime[2][1], transformAPrime[2][2]);
  Serial.println("");

  float transformBRaw[3][4] = {{(float)_VX(finders[1]), (float)_VX(finders[2]), (float)_VX(finders[0]), (float)_VX(anchor)},
                               {(float)_VY(finders[1]), (float)_VY(finders[2]), (float)_VY(finders[0]), (float)_VY(anchor)},
                               {                    1.,                     1.,                     1.,                 1.}};

  Serial.printf("scanQR:      [%.3f, %.3f, %.3f | %.3f]\n", transformBRaw[0][0], transformBRaw[0][1], transformBRaw[0][2], transformBRaw[0][3]);
  Serial.printf("scanQR: Br = [%.3f, %.3f, %.3f | %.3f]\n", transformBRaw[1][0], transformBRaw[1][1], transformBRaw[1][2], transformBRaw[1][3]);
  Serial.printf("scanQR:      [%.3f, %.3f, %.3f | %.3f]\n", transformBRaw[2][0], transformBRaw[2][1], transformBRaw[2][2], transformBRaw[2][3]);
  Serial.println("");

  float *transformBScale = gaussReduce(transformBRaw);

  Serial.printf("scanQR: coefs = [%.3f, %.3f, %.3f]\n", transformBScale[0], transformBScale[1], transformBScale[2]);
  Serial.println("");

  float transformB[3][3] = {{(float)_VX(finders[1])*transformBScale[0], (float)_VX(finders[2])*transformBScale[1], (float)_VX(finders[0])*transformBScale[2]},
                            {(float)_VY(finders[1])*transformBScale[0], (float)_VY(finders[2])*transformBScale[1], (float)_VY(finders[0])*transformBScale[2]},
                            {                       transformBScale[0],                        transformBScale[1],                        transformBScale[2]}};

  Serial.printf("scanQR:     [%.3f, %.3f, %.3f]\n", transformB[0][0], transformB[0][1], transformB[0][2]);
  Serial.printf("scanQR: B = [%.3f, %.3f, %.3f]\n", transformB[1][0], transformB[1][1], transformB[1][2]);
  Serial.printf("scanQR:     [%.3f, %.3f, %.3f]\n", transformB[2][0], transformB[2][1], transformB[2][2]);
  Serial.println("");
  
  for (i = 0; i < 3; i++) {
    for (j = 0; j < 3; j++) {
      perspective_transform[i][j] = 0.;
      for (n = 0; n < 3; n++) {
        perspective_transform[i][j] += transformB[i][n] * transformAPrime[n][j];
      }
    }
  }

  Serial.printf("scanQR:     [%.3f, %.3f, %.3f]\n", perspective_transform[0][0], perspective_transform[0][1], perspective_transform[0][2]);
  Serial.printf("scanQR: T = [%.3f, %.3f, %.3f]\n", perspective_transform[1][0], perspective_transform[1][1], perspective_transform[1][2]);
  Serial.printf("scanQR:     [%.3f, %.3f, %.3f]\n", perspective_transform[2][0], perspective_transform[2][1], perspective_transform[2][2]);
  Serial.println("");
  
  Serial.printf("scanQR: T = {{%.3f, %.3f, %.3f},{%.3f, %.3f, %.3f},{%.3f, %.3f, %.3f}}\n", 
    perspective_transform[0][0], perspective_transform[0][1], perspective_transform[0][2],
    perspective_transform[1][0], perspective_transform[1][1], perspective_transform[1][2],
    perspective_transform[2][0], perspective_transform[2][1], perspective_transform[2][2]);
  Serial.println("");

  Serial.printf("scanQR: transforming 4,4 (should be => %d,%d)\n", _VX(finders[0]), _VY(finders[0]));
  applyTransform(4, 4);
  
  Serial.printf("scanQR: transforming anchor (should be => %d,%d)\n", _VX(anchor), _VY(anchor));
  applyTransform(QR_N - 4, QR_N - 4);
}

void processQRCode(uint8_t *buf, int width) {
  int i, j;

  for (i = 0; i < QR_N; i++) {
    for (j = 0; j < QR_N; j++) {
      applyTransform(i, j);
      rawCode[j][i] = IS_BLACK(buf[_VX(transformed_coords) + _VY(transformed_coords) * width]);
    }
  }

  for (i = 0; i < QR_N; i++) {
    for (j = 0; j < QR_N; j++) {
      Serial.print(rawCode[j][i] ? "█" : "  ");
    }
    Serial.println("");
  }
}

bool verifyTimingPattern() {
  int n;
  uint8_t black = 1;

  for (n = QR_TIMING_START; n < QR_N - QR_TIMING_START; n++) {
    Serial.printf("scanQR: timing @ %d,%d should be %d, is actually %d\n", QR_TIMING_START, n, (int)black, rawCode[QR_TIMING_START][n]);
    if ((black) ^ (rawCode[n][QR_TIMING_START])) {
      Serial.printf("scanQR: timing @ %d,%d should be %d, is actually %d\n", QR_TIMING_START, n, (int)black, rawCode[QR_TIMING_START][n]);
      return false;
    }
    
    Serial.printf("scanQR: timing @ %d,%d should be %d, is actually %d\n", n, QR_TIMING_START, (int)black, rawCode[QR_TIMING_START][n]);
    if ((black) ^ (rawCode[QR_TIMING_START][n])) {
      Serial.printf("scanQR: timing @ %d,%d should be %d, is actually %d\n", n, QR_TIMING_START, (int)black, rawCode[QR_TIMING_START][n]);
      return false;
    }

    black = 1 - black;
  }

  Serial.println("scanQR: timing successful!");
  return true;
}

int scanQR(uint8_t **target) {
   camera_fb_t * fb = NULL;
   int x, y;
  
#ifdef DEBUG
  Serial.println("");
  Serial.printf("scanQR:==========================================\n");
  Serial.printf("scanQR: start\n");
#endif

  total_finders = 0;
  memset(finders, 0, 3*MAX_FINDERS*sizeof(int));

  fb = esp_camera_fb_get();
  if (fb == NULL) {
    Serial.println("Could not get picture!");
    *target = NULL;
    return -1;
  }

#ifdef DEBUG
  Serial.printf("scanQR: %dx%d\n", fb->width, fb->height);
#endif

#ifdef DEBUG_QR_IMAGE
  Serial.println("");
  Serial.println("=========================================================================================");
  for (y=0;y<fb->height;y++) {
    for (x=0;x<fb->width;x++) {
      Serial.print(IS_BLACK(fb->buf[x + y * fb->width]) ? "#" : " ");
    }
    Serial.println("");
  }
  Serial.println("=========================================================================================");
  Serial.println("");
#endif

  if (!findFinders(fb->buf, fb->width, fb->height)) {
    *target = NULL;
    return -1;
  }
  
  // TODO realign (persp. transform)
  reorderFinders();
  getAnchor();
  calculatePerspective();

  Serial.println("");
  Serial.println("=========================================================================================");
  for (y=0;y<fb->height;y++) {
    for (x=0;x<fb->width;x++) {
      Serial.print(IS_BLACK(fb->buf[x + y * fb->width]) ? "█" : "  ");
    }
    Serial.println("");
  }
  Serial.println("=========================================================================================");
  Serial.println("");

  processQRCode(fb->buf, fb->width);
  
  if (!verifyTimingPattern()) {
    Serial.println("scanQR: timing failure");
    *target = NULL;
    return -1;
  }

  // TODO read data
  
  *target = NULL;
  return 0;
}

#endif
