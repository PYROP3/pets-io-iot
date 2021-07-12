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

#define MASK_ODD_STATE 0x1

#define MAX_FINDERS 5

//int finders[3][MAX_FINDERS]; // -> [x][y][size]
int finders[MAX_FINDERS][3]; // -> [x][y][size]
int total_finders = 0;

typedef struct {
  String registerToken;
  String ap_ssid;
  String ap_password;
} parsed_json_t;

int findCenter(int *ratio, int ratioEnd) {
  return ratioEnd - ratio[4] - ratio[3] - (ratio[2]/2);
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
  
  if (curr_y<0 || checkStates[1]>=ratioMiddle) { // Not a center (out of image or center is smaller than rest)
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
  
  if (curr_y<0 || checkStates[0]>=ratioMiddle) { // Not a center (out of image or center is smaller than rest)
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
  
  if (curr_y == max_y || checkStates[3]>=ratioMiddle) { // Not a center (out of image or center is smaller than rest)
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
  
  if (curr_y == max_y || checkStates[4]>=ratioMiddle) { // Not a center (out of image or center is smaller than rest)
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
  int estimated_size = total_size / 7;
  
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
  estimated_size = total_size/7;
  found = false;
  for (finder_idx = 0; finder_idx < total_finders; finder_idx++) {
    if (distSquared(finders[finder_idx][0], finders[finder_idx][1], center_x, center_y) < 100) { // Too close, same finder
      finders[finder_idx][0] = (finders[finder_idx][0] + center_x) / 2;
      finders[finder_idx][1] = (finders[finder_idx][1] + center_y) / 2;
      finders[finder_idx][2] = (finders[finder_idx][2] + estimated_size) / 2;
#ifdef DEBUG
      Serial.printf("scanQR: Found duplicated finder [%d], updating (%d, %d, %d) => (%d, %d, %d)\n", finder_idx, center_x, center_y, estimated_size, finders[finder_idx][0],finders[finder_idx][1], finders[finder_idx][2]);
#endif
      found = true;
      break;
    }
  }

  if (!found) { // Found new finder!
#ifdef DEBUG
    Serial.printf("Found new finder!\n");
#endif
    finders[total_finders][0] = center_x;
    finders[total_finders][1] = center_y;
    finders[total_finders][2] = estimated_size;
    total_finders++;
  }

  return true; // should be true?? // was false
}

parsed_json_t* scanQR() {
  int x, y;
  boolean confirmed;
  int ratio[5];
  int ratio_ptr = 0;
  camera_fb_t * fb = NULL;
  // parsed_json_t * json = (parsed_json_t*)malloc(sizeof(parsed_json_t));
  
#ifdef DEBUG
  Serial.println("");
  Serial.printf("scanQR:==========================================\n");
  Serial.printf("scanQR: start\n");
#endif

  total_finders = 0;
  memset(finders, 0, 3*MAX_FINDERS*sizeof(int));

  fb = esp_camera_fb_get();

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

  for (y=0;y<fb->height;y++) {
    memset(ratio, 0, 5*sizeof(int));
    ratio_ptr = 0;
    for (x=0;x<fb->width;x++) {
      if (IS_BLACK(fb->buf[x + y * fb->width])) { // If in black
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
              confirmed = handleFinder(fb->buf, ratio, x, y, fb->width, fb->height);
#ifdef DEBUG
              if (confirmed) {
                Serial.printf("scanQR: CONFIRMED finder @ (%d,%d)\n", x, y);
              } else {
                Serial.printf("scanQR: false positive @ (%d,%d)\n", x, y);
              }
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

  Serial.printf("scanQR: found %d finders\n", total_finders);
  for (ratio_ptr = 0; ratio_ptr < total_finders; ratio_ptr++) {
    Serial.printf("scanQR:\t(%d, %d, %d)\n", finders[ratio_ptr][0], finders[ratio_ptr][1], finders[ratio_ptr][2]);
  }

#ifdef DEBUG
  Serial.printf("scanQR: finished!\n");
#endif

  // TODO parse QR code & extract data
  return NULL;
}

#endif
