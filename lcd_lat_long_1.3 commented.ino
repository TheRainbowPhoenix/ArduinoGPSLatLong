#include <LiquidCrystal.h> // LCD Library
#include <string.h> // String type library
#include <ctype.h> // another type library

LiquidCrystal lcd(12, 11, 5, 4, 3, 2); // LCD Pin configuration

int rxPin = 0; // RX pin
int txPin = 1; // TX pin
int byteGPS=-1; // GPS bytes, Empty by default
char cmd[7] = "$GPRMC"; // We're working with RMC frame
int RcvdCnt = 0; // counts how many bytes were received (max 300)
int CommaCount = 0; // counts how many commas were seen
int offsets[13];
char buf[300] = "";

/*
 * Setup display and gps
 */
void setup() {
  pinMode(rxPin, INPUT); // Configure rxPin Variable as Input port
  pinMode(txPin, OUTPUT); // Configure txPin Variable as Output port
  Serial.begin(9600); // Baud rate
  lcd.begin(16, 2); // LCD configuration(2 lines, 16 char long)
  lcd.print("Waiting for GPS"); // Welcome message
  offsets[0] = 0; // Offset List Configuration
  reset(); // Call reset function
}

void reset() {
  RcvdCnt = 0; // Reset Received bytes counter
  CommaCount = 0; // Reset commas counter
}

int get_size(int offset) {
  return offsets[offset+1] - offsets[offset] - 1; // Super Ninja technique for counting offsets size
}

int handle_byte(int byteGPS) {
  buf[RcvdCnt] = byteGPS;  // Attribute to the byteGPS variable the buffer of the received bytes 
  Serial.print((char)byteGPS); // Print on serial port byteGPS, so buf[RcvdCnt] .
  RcvdCnt++; // Increment Received counter
  if (RcvdCnt == 300) {
    return 0; // If it's over -9000- the buffer size, break
  }
  if (byteGPS == ',') {
    CommaCount++; // Increment Commas Counter
    offsets[CommaCount] = RcvdCnt; // Attribute to RcvdCnt the values between the commas
    if (CommaCount == 13) {
      return 0; // If commas are over 13, it mean that frame is invalid (corrupted ?)
    }
  }
  if (byteGPS == '*') {
    offsets[12] = RcvdCnt; // The Star is not a lie, it's the end of the line
  }

  // Check if we got a <LF>, which indicates the end of line
  if (byteGPS == 10) {
    // Check that we got 12 pieces, and that the first piece is 6 characters
    if (CommaCount != 12 || (get_size(0) != 6)) {
      return 0;
    }

    // Check that we received $GPRMC
    for (int j=0; j<6; j++) {
      if (buf[j] != cmd[j]) { // Compare if the 6 firsts characters are different from the one set one the cmd
        return 0;
      }
    }

    // Check that time is well formed
    if (get_size(1) != 10) { // Time should be a 10 characters block, if not we just break
      return 0;
    }

    // Check that latitude is well formed
    // considering that minimal GPS latitude data frame is 0.0000, so 6 characters
    // the maximal latitude is about 180°00.0000, so 10 characters
    if (get_size(3) != 9) {
      return 0;
    }

    // Check that longitude is well formed
    // considering that minimal GPS longitude data frame is 0.0000, so 6 characters
    // the maximal longitude is about 180°00.0000, so 10 characters
    if (get_size(5) != 10) {
      return 0;
    }

    // Check that date is well formed
    if (get_size(9) != 6) { // Date should be a 6 characters block, if not we just break
      return 0;
    }
    
    // TODO: Integrity check
    
    // TODO: compute and validate checksum

    // TODO: handle timezone offset

    // print time
    
    /*
    
    lcd.clear();
    for (int j=0; j<6; j++) {
      lcd.print(buf[offsets[1]+j]);
      if (j==1) {
        lcd.print("h");
      } else if (j==3) {
        lcd.print("m");
      } else if (j==5) {
        lcd.print("s UTC");
      }
    }
    
    */

    // print date
    
    /*
    
    lcd.setCursor(0, 1);
    for (int j=0; j<6; j++) {
      lcd.print(buf[offsets[9]+j]);
      if (j==1 || j==3) {
        lcd.print(".");
      }
    }
    
    */
    
    // print latitude
    
	/*
	 * How Algorithm works :
	 *
	 * We divide the offsets[3] which contains latitude in two parts
	 * First it's the part before the . (the 6 first characters) which is divided in two : ° and , 
	 * Then we take the second block and display a ' after it 
	 * To get the block length, we just take the length the block after - the block before = the block size
	 */
	
	
    lcd.clear(); // Clear LCD (Because the welcome message is still here
    for (int j=0; j<(offsets[4]-offsets[3]-6); j++) {
      lcd.print(buf[offsets[3]+j]); // Here it's the Super Ninja Technikz II (comment above)
      if (j==1) {
        lcd.print("o");
      } else if (j==3) {
        lcd.print(",");
      }
    }
    for (int j=5; j<(offsets[4]-offsets[3]-1); j++) {
      lcd.print(buf[offsets[3]+j]);
      if (j==8) {
        lcd.print("'");
      }
    }
    
    // print longitude
    
    //diable "0" if exists
    
    int Zero = 0;
    
    if (buf[offsets[5]]= 0) {
      int Zero = 1;  
    }
    
    lcd.setCursor(0, 1);
    for (Zero; Zero<(offsets[6]-offsets[5]-6); Zero++) {
      lcd.print(buf[offsets[5]+Zero]);
      if (Zero==2) {
        lcd.print("o");
      } else if (Zero==4) {
        lcd.print(",");
      }
    }
    for (int j=6; j<(offsets[6]-offsets[5]-1); j++) {
      lcd.print(buf[offsets[5]+j]);
      if (j==9) {
        lcd.print("'");
      }
    }
      
    return 0;
  }
  return 1;
}

/**
 * Main loop
 */
void loop() {
  byteGPS=Serial.read();         // Read a byte of the serial port
  if (byteGPS == -1) {           // See if the port is empty yet
    delay(100);
  } else {
    if (!handle_byte(byteGPS)) {
      reset();
    }
  }
}