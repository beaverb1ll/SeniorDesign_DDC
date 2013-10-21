#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
#include <SD.h>

#define CUP_PRESENT_THRESHOLD 600   // this holds the voltage reading that a cup is considered present
#define SERIAL_READ_TIMEOUT 100     // (ms)
#define CUP_PRESENT_TIMEOUT 100     // (ms)

#define FALSE 0
#define TRUE 1
#define OFF HIGH
#define ON LOW

#define SD_CS   10  // Chip select line for SD card
#define TFT_CS  6  // Chip select line for TFT display
#define TFT_DC  7  // Data/command line for TFT
#define TFT_RST  8  // Reset line for TFT (or connect to +5V)

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

boolean isDispensing;   // holds dispensing status
boolean isCupDetected;  // holds cup detection status
boolean timerValid;     // holds whether timer has overfload and passed target value

int ingredients[6];     // holds amount of dipense time for respective ingredients
int currentIngred;      // holds the current ingredient that is being dispensed
int amountDispensed;    // holds the amount dispensed if interupted
int timerValue;         // hods the amount of time (in ms) that has passed during the timer
int targetValue;        // holds the amount of time (in ms) that the timer should run for

int pins[6];            // Allows for easy changing and referencing of pins

void setup(void) {

    // initialize pins
    pins[0] = ;
    pins[1] = ;
    pins[2] = ;
    pins[3] = ;
    pins[4] = ;
    pins[5] = ;

    // set pins to output
    for (int i = 0; i < NUM_INGREDIENTS; i++)
    {
        pinMode(pins[i], OUTPUT);
    }

    // configure serial port
    Serial.begin(9600);

    // initialize display
    tft.initR(INITR_BLACKTAB);
    if (!SD.begin(SD_CS)) {
        Serial.println("DEBUG :: Error init screen");
        return;
    }

    tft.fillScreen(ST7735_BLACK); // Clear display

    // configure cup sensor 

}



void loop(void) {
   // update display
    bmpDraw("screen0.bmp", 0, 0);

    // initialize dispense variables
    isDispensing = false;
    isCupDetected = false;

   // wait for read incoming character
    while (Serial.available() == 0)
   /* just wait */ ;

    /* read the incoming byte */
    int input = Serial.read();

   // test for operation
    if (input != "D")
    {
        continue;
    }
   // read in ingredients and store into array
   if(!readIngredients()) {
        continue;
    }

   // wait for cup
    if (analogRead(A0) > CUP_PRESENT_THRESHOLD)
    {
        // cup is not detected
        bmpDraw("screen1.bmp", 0, 0);

        startTimer(CUP_PRESENT_TIMEOUT);

        // set a timer to see if user waits too long
        while(!isCupDetected && timerValid)   // timerValid is updated when timer expires
        {
            // check again. if valid, continue
            if(analogRead(A0) <= CUP_PRESENT_THRESHOLD) 
            {
                Timer1.stop();
                isCupDetected = true;
            }
        }
        if (!timerValid)
        {
            if(!sendcommand_getAck('N')) {
                // handle the error somehow
            }
            continue; // start entire dispensing process order
        }
    }

   // update display
    bmpDraw("screen2.bmp", 0, 0);
   // loop through all ingredients
    int i;
    for (i = 0; i < NUM_INGREDIENTS; i++)
    {
        if(!startDispensing(i, ingredients[i]))
        {
            // stop on failure
            break;
        }
    }

    if (i == NUM_INGREDIENTS) // all ingredients dispensed
    {
         if(!sendcommand_getAck('Z')) {
        // handle the error somehow
        }
    } else
    {
         if(!sendcommand_getAck('N')) {
        // handle the error somehow
        }

    }
  
}

boolean sendCommnad_getAck(char aCommand) {
    // send aCommand over serial and wait for a response

    startTimer(SERIAL_READ_TIMEOUT);
    while (Serial.available() == 0 && timerValid)
          ;  /* just wait */
    
    Timer1.stop();

    if (!timerValid)
    {
        return false;
    }

    int temp = Serial.read();
    if (temp == 'Y')
    {
        return true;
    }
    return false;
}

void timerInterrupt(void) 
{
    if(timerValue > targetValue) {
        timerValue = 0;
        timerValid = false;
        Timer1.stop();
    } else 
    {
        timerValue++;
    }
}

boolean readIngredients(void) {
    // this function is responsible for populating the array that holds the ingredient amounts.
    
    for (int i = 0; i < NUM_INGREDIENTS; i++)
    {
        startTimer(SERIAL_READ_TIMEOUT);

        while (Serial.available() == 0 && timerValid)
            ; /* just wait */ 

        Timer1.stop();

        if (!timerValid)
        {
            return false;
        }

        // read in value, store them into ints.
        int temp = Serial.read();
        ingredients[i] = temp;
    }

    return true;
    // it will return true if all ingredients are read correctly. 

    // it will return false if something goes wrong.
}


boolean checkforCup(void)
{
    while (isDispensing) 
    {
        if (analogRead(A0) > CUP_PRESENT_THRESHOLD) // cup is no longer detected
        {
            // this will occur if voltage drops below some value.
            isCupDetected = false;

            // stop timer
            Timer1.stop();

            // get time passed so far from timer
            // save it to amountDispensed Variable
            amountDispensed = timerValue;
            
            // stop dispensing of currentIngred
            digitalWrite(pins[currentIngred], OFF);

            // enable walk away timer
            startTimer(CUP_PRESENT_TIMEOUT);

            while (analogRead(A0) > CUP_PRESENT_THRESHOLD && timerValid) 
                ; /*just wait*/

            if (!timerValid)
            {
                return false;
            }
            isCupDetected = true;

            // restore timer value
            timerValue = amountDispensed;
            startTimer(ingredients[currentIngred]);
            digitalWrite(pins[currentIngred], ON);
        }
    }
    return true;
}

boolean startDispensing(int aIngred, int aTime) {
    // this will take in a ingredient value and a time value.
    // the ingredient value specifies the ingreient slot that will dispense with
    // the time value will specify the amount of time the ingredient will be opened.

    // This function will be responsible for:
    //  set interupt timer value as aTime


    //  set dispensing equal to true
    isDispensing = true;

    // initialize amount dispensed
    amountDispensed = 0;

    //  start timer
    startTimer(aTime);

    //  change output value to DISPENSE on pin for aIngre
    digitalWrite(pins[currentIngred], ON);

    //  ensure cup stays present. isDispensing will get changed by timer interrupt
    if(!checkforCup()) 
    {
        return false;
    }

    //  stop dispensing on current slot
    digitalWrite(pins[currentIngred], OFF);
    isDispensing = false;
    return true;
}

void startTimer(int aTime) 
{
    timerValid = true;
    targetValue = aTime;
    Timer1.initialize(1000);
}



// This function opens a Windows Bitmap (BMP) file and
// displays it at the given coordinates.  It's sped up
// by reading many pixels worth of data at a time
// (rather than pixel by pixel).  Increasing the buffer
// size takes more of the Arduino's precious RAM but
// makes loading a little faster.  20 pixels seems a
// good balance.

#define BUFFPIXEL 20

void bmpDraw(char *filename, uint8_t x, uint8_t y) {

  File     bmpFile;
  int      bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t rowSize;               // Not always = bmpWidth; may have padding
  uint8_t  sdbuffer[3*BUFFPIXEL]; // pixel buffer (R+G+B per pixel)
  uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  int      w, h, row, col;
  uint8_t  r, g, b;
  uint32_t pos = 0, startTime = millis();

  if((x >= tft.width()) || (y >= tft.height())) return;

  Serial.println();
  Serial.print("Loading image '");
  Serial.print(filename);
  Serial.println('\'');

  // Open requested file on SD card
  if ((bmpFile = SD.open(filename)) == NULL) {
    Serial.print("File not found");
    return;
  }

  // Parse BMP header
  if(read16(bmpFile) == 0x4D42) { // BMP signature
    Serial.print("File size: "); Serial.println(read32(bmpFile));
    (void)read32(bmpFile); // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
    Serial.print("Image Offset: "); Serial.println(bmpImageoffset, DEC);
    // Read DIB header
    Serial.print("Header size: "); Serial.println(read32(bmpFile));
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if(read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
      Serial.print("Bit Depth: "); Serial.println(bmpDepth);
      if((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed

        goodBmp = true; // Supported BMP format -- proceed!
        Serial.print("Image size: ");
        Serial.print(bmpWidth);
        Serial.print('x');
        Serial.println(bmpHeight);

        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * 3 + 3) & ~3;

        // If bmpHeight is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if(bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip      = false;
        }

        // Crop area to be loaded
        w = bmpWidth;
        h = bmpHeight;
        if((x+w-1) >= tft.width())  w = tft.width()  - x;
        if((y+h-1) >= tft.height()) h = tft.height() - y;

        // Set TFT address window to clipped image bounds
        tft.setAddrWindow(x, y, x+w-1, y+h-1);

        for (row=0; row<h; row++) { // For each scanline...

          // Seek to start of scan line.  It might seem labor-
          // intensive to be doing this on every line, but this
          // method covers a lot of gritty details like cropping
          // and scanline padding.  Also, the seek only takes
          // place if the file position actually needs to change
          // (avoids a lot of cluster math in SD library).
          if(flip) // Bitmap is stored bottom-to-top order (normal BMP)
            pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
          else     // Bitmap is stored top-to-bottom
            pos = bmpImageoffset + row * rowSize;
          if(bmpFile.position() != pos) { // Need seek?
            bmpFile.seek(pos);
            buffidx = sizeof(sdbuffer); // Force buffer reload
          }

          for (col=0; col<w; col++) { // For each pixel...
            // Time to read more pixel data?
            if (buffidx >= sizeof(sdbuffer)) { // Indeed
              bmpFile.read(sdbuffer, sizeof(sdbuffer));
              buffidx = 0; // Set index to beginning
            }

            // Convert pixel from BMP to TFT format, push to display
            b = sdbuffer[buffidx++];
            g = sdbuffer[buffidx++];
            r = sdbuffer[buffidx++];
            tft.pushColor(tft.Color565(r,g,b));
          } // end pixel
        } // end scanline
        Serial.print("Loaded in ");
        Serial.print(millis() - startTime);
        Serial.println(" ms");
      } // end goodBmp
    }
  }

  bmpFile.close();
  if(!goodBmp) Serial.println("BMP format not recognized.");
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(File f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}
