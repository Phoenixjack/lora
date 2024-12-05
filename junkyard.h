
#define HC12 Serial

const char bracket = '$';    // used for marking beginning and end of transmission
const char separator = '#';  // used for separating parts of a message header, NOT PAYLOAD. Payload will be CSV
const char id1 = 'G';        // first character of my unique identifier. Used to tag outgoing transmissions and filter incoming messages
const char id2 = 'X';        // second character of my unique identifier

struct lora {
  bool init = false;          // is it initialized?
  bool cfgmode = false;       // for SET pin to enter config mode
  bool cfgtimeout;            // for use in CONFIG timeout loops
  bool cfgresponse;           // for use in CONFIG timeout loops
  long speed = 9600;          // Available baud rates: 1200 bps, 2400 bps, 4800 bps, 9600 bps, 19200 bps, 38400 bps, 57600 bps, and 115200 bps. Default: 9600 bps
  uint8_t chan = 0;           // 001 to 100. Default: Channel 001, with working frequency of 433.4MHz. Each next channel is 400KHz higher.
  uint8_t power = 8;          // 1: -1dBm, 2: 2dBm/1.6mW, 3: 5dBm/3.2mW, 4: 8dBm/6mW, 5: 11dBm/13mW, 6: 14dBm/25mW, 7: 17dBm/50mW, 8: 20dBm/100mW
  uint8_t fmode = 3;          // 1-3. Change serial port transparent transmission mode
  uint16_t cfgtimeout_ms;     // amount of ms to wait for response when in CONFIG mode
  unsigned long pingsent;     // timestamp of when we send a ping; used to determine when we get a response
  String rcvtext;             // buffer to hold incoming lora data
  char xmtbuffer[50];         // reserve space for our outgoing message
  uint8_t xmtbufflength = 0;  // for tracking how full the buffer is
} lora;

struct report {
  bool enabled = true;                     // enabled by default
  unsigned long interval = 500;            // default auto report interval in milliseconds
  unsigned long nextsend_ms;               // to save loop time, we'll precompute the minimum millis before we can check the sensor again
  const unsigned long mininterval = 200;   // min milliseconds
  const unsigned long maxinterval = 3000;  // max milliseconds
  uint32_t msgcount = 0;                   // tracks number of outgoing data reports; DOES NOT ACCOUNT FOR DIAGNOSTIC AND COMMAND traffic
} report;

bool init_lora() {
  lora.init = false;
  HC12.begin(9600);
  delay(100);
  /*
  lora.cfgtimeout = false;
  lora.cfgresponse = false;
  String diagtext;
  digitalWrite(PINHC12SET, LOW);
  lora.cfgmode = true;
  HC12.println("AT");
  // ... timeout...
  while (!lora.cfgtimeout && !lora.cfgresponse) {
    lcd.setCursor(0, 0);
    lcd.print(millis() - timer);
    if (HC12.available()) {
      diagtext = HC12.readString();
      if (diagtext.startsWith("OK")) { lora.cfgresponse = true; }
    }
    if (millis() > timer + 10000) { lora.cfgtimeout = true; }
  }
  lora.init = lora.cfgresponse & !lora.cfgtimeout;
  lcd.clear();
  lcd.setCursor(0, 0);
  if (lora.init) {
    lcd.print("LORA INITIALIZED");
    HC12.println("UNOAS5600LORA BOOTED!");
  } else if (lora.cfgtimeout) {
    lcd.print("LORA TIMEOUT");
  } else {
    lcd.println("LORA INIT:");
    lcd.println("UNKNOWN ERROR");
  }
  digitalWrite(PINHC12SET, HIGH);
  lora.cfgmode = false;
  */
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("LORA INITIALIZED"));
  lora.init = true;
  return lora.init;
}

void cmd_loratransmit(char* purpose, String* xmttext) {
  HC12.print(bracket);     // begin message
  HC12.print(id1);         // my 2 position ID
  HC12.print(id2);         //
  HC12.print(separator);   // denote new column for header
  HC12.print(*purpose);    // what's this message about?
  HC12.print(separator);   // denote new column for header
  HC12.println(*xmttext);  // actual payload. will be CSV
}

void cmd_sendbasicreport() {                        // sends read time of Angle in usecs, msg counter, Angle
  report.msgcount++;                                // increment counter
  String strXmitMsg = String(az.lastread_us);       // columns=micros,msgcnt,angle
  strXmitMsg += "," + String(report.msgcount);      // add msgcount
  strXmitMsg += "," + String(az.ANGLE);             // add angle (0-4096) (corrected for direction and offset)
  char purpose = 'D';                               //
  cmd_loratransmit(&purpose, &strXmitMsg);          // send the report
  report.nextsend_ms = millis() + report.interval;  // set the minimum time before we can send another report
}

void cmd_processinput(String* rcvtext) {              // act on commands received from Serial or HC12. **TO SAVE RAM**, we'll reuse rcvtext as payload for repeatback
  int rcvlength = rcvtext->length();                  // get the length for initial validation
  char purpose;                                       // subroutines will mark the return package for speedy processing
  if (rcvlength > 3) {                                // if it's not long enough, don't even bother
    char rid1 = rcvtext->charAt(0);                   // extract the intended target ID
    char rid2 = rcvtext->charAt(1);                   // ""
    char intent = rcvtext->charAt(2);                 // and the intent (query, command, set configuration)
    bool success = false;                             // flag that will be set to true if command is successfully executed
    if (rid1 == id1 && rid2 == id2) {                 // check if the target ID is us
      switch (intent) {                               // if so, process the intent
        case 'Q':                                     // it's a query
          success = proc_query(rcvtext, &purpose);    // process it. function will return T/F and overwrite payload with success or error message
          break;                                      // exit switch case so we can process the reply
        case 'C':                                     // it's a direct command
          success = proc_command(rcvtext, &purpose);  // process it. function will return T/F and overwrite payload with success or error message
          break;                                      // exit switch case so we can process the reply
        case 'S':                                     // we're supposed to apply a setting
          success = proc_set(rcvtext, &purpose);      // process it. function will return T/F and overwrite payload with success or error message
          break;                                      // exit switch case so we can process the reply
        default:                                      // no match; error
          purpose = 'E';                              //
          break;                                      // exit switch case so we can process the reply
      };                                              // end 'intent' switch
      cmd_loratransmit(&purpose, rcvtext);            // send the report
    } else {                                          // do nothing. the target ID doesn't match ours
    }                                                 // end ID match
  }                                                   // end rcvlength check
}

bool proc_command(String* rcvtext, char* purpose) {  // process commands received by LORA or Serial: ACKLMPQSZ
  char target = rcvtext->charAt(3);                  // extract the 'target' of our command
  *purpose = 'A';                                    // mark return message as acknowledgement
  switch (target) {                                  // check if 'target' character matches a valid target
    case 'A':                                        //Command: A Stop rotation
      cmd_relayset(false);                           // call routine to execute
      *rcvtext = F("rotation,stopped");              // overwrite with feedback
      return true;                                   // report success
    case 'C':                                        //Command: C reset msgcount
      report.msgcount = 0;                           // reset the count
      *rcvtext = F("count reset");                   //
      return true;                                   //
    case 'K':                                        //Command: K LCD off
      disp.on = false;                               //
      *rcvtext = F("LCD off");                       //
      return true;                                   //
    case 'L':                                        //Command: L LCD on
      disp.on = true;                                //
      *rcvtext = F("LCD on");                        //
      return true;                                   //
    case 'M':                                        //Command: P Start regular reports
      *rcvtext = F("inertia test started");          // load notification
      cmd_loratransmit(purpose, rcvtext);            // send notification
      cmd_lead_test();                               // execute
      *rcvtext = F("inertia test complete");         //
      return true;                                   //
    case 'P':                                        //Command: P Start regular reports
      report.enabled = true;                         // set flag
      *rcvtext = F("autoreport,enabled");            //
      return true;                                   //
    case 'Q':                                        //Command: Q Stop regular reports
      report.enabled = false;                        // set flag
      *rcvtext = F("autoreport,disabled");           //
      return true;                                   //
    case 'R':                                        //Command: R Reverse rotation reading
      cmd_reversedirection();                        // call routine to execute
      *rcvtext = F("as5600,direction reversed");     //
      return true;                                   //
    case 'S':                                        //Command: S Start rotation
      cmd_relayset(true);                            // call routine to execute
      *rcvtext = F("rotation,started");              //
      return true;                                   //
    case 'W':                                        //Set: W Set CW
      *rcvtext = F("CW set");                        //
      return cmd_setCW();                            // report success or failure
    case 'V':                                        //Set: V Set CCW
      *rcvtext = F("CCW set");                       //
      return cmd_setCCW();                           // report success or failure
    case 'Z':                                        //Command: Z Set current position as zero
      cmd_setzero();                                 // do it
      *rcvtext = F("zero set");                      //
      return true;                                   //
    default:                                         // no match to known commands
      *rcvtext = F("parsecommand,no match");         // set payload to error message
      *purpose = 'E';                                // mark return message as error
      return false;                                  // report failure
  };                                                 // end command switch
  return false;                                      // theoretically, we should NEVER reach this, but just in case
}

bool proc_query(String* rcvtext, char* purpose) {      // process QUERIES received by LORA or Serial
  char target = rcvtext->charAt(3);                    // extract the 'target' of our QUERY
  switch (target) {                                    // check if 'target' character matches a valid target
    case 'D':                                          //Query: D extended node details. Compile date/time, Software & Version, report interval (JSON)
      cmd_getnodedetails(rcvtext);                     // overwrites rcvtext with JSON data
      *purpose = 'J';                                  // mark the package as JSON
      return true;                                     // report success
    case 'E':                                          //Query: E extended sensor details: AGC, Magnitude, RPM (JSON)
      cmd_getsensordetails(rcvtext);                   // overwrites rcvtext with JSON data
      *purpose = 'J';                                  // mark the package as JSON
      return true;                                     // report success
    case 'H':                                          //Query: H column headers for basic report
      *rcvtext = F("micros,msgcount,angle");           // overwrite with feedback
      *purpose = 'H';                                  // mark this as a column header response
      return true;                                     // report success
    case 'K':                                          //Query: K valid commands in shorthand
      *rcvtext = F("Q(DEHM),C(CSARPQZKL),S(ITO)int");  // load it
      *purpose = 'R';                                  // mark return package as a report
      return true;                                     // send it
    case 'M':                                          //Query: M current micros. Useful for syncing server with node time
      *rcvtext = String(micros());                     // overwrite with feedback
      *purpose = 'T';                                  // mark return package as time
      return true;                                     // report success
    default:                                           // no match to known commands
      *rcvtext = F("parsequery,no match");             // set payload to error message
      *purpose = 'E';                                  // mark return message as error
      return false;                                    // report failure
  };                                                   // end QUERY switch
  return false;                                        // theoretically, we should NEVER reach this, but just in case
}

bool proc_set(String* rcvtext, char* purpose) {  // process SET CONFIG received by LORA or Serial
  char target = rcvtext->charAt(3);              // extract the 'target' of our SET
  *rcvtext = rcvtext->substring(4);              // isolate the numeric portion of our message
  *purpose = target;                             // save code and set the reply character to what we got
  switch (target) {                              // check if 'target' character matches a valid target
    case 'I':                                    //Set: I report interval in msec
      return set_newinterval(rcvtext);           // report success or failure
    case 'O':                                    //Set: O Offset in raw (0-4096)
      return set_offset(rcvtext);                // report success or failure
    case 'S':                                    //Set: O Offset in raw (0-4096)
      return cmd_seekazimuthstr(rcvtext);        // report success or failure
    case 'T':                                    //Set: T Manual time lag in msec. Useful if you wanted to try syncing regular reports with data packets.
      return set_manuallag(rcvtext);             // report success or failure
    default:                                     // no match to known commands
      *rcvtext = F("parseset,no match");         // set payload to error message
      *purpose = 'E';                            // mark return message as error
      return false;                              // report failure
  };                                             // end SET switch
  return false;                                  // theoretically, we should NEVER reach this, but just in case
}

void cmd_getnodedetails(String* strXmitMsg) {  // sends extended node details like software, version, compile date/time, and report interval
  //JSONVar objData;
  //objData["millis"] = millis();
  //objData["msgcount"] = report.msgcount;
  //objData["interval"] = report.interval;
  //objData["Software"] = defSoftwareName;
  //objData["Version"] = defSoftwareVer;
  //objData["CompDate"] = defCompDate;
  //objData["CompTime"] = defCompTime;
  //*strXmitMsg = JSON.stringify(objData);
}

bool set_newinterval(String* payload) {                                // convert payload to integer, validate, and apply it if good
  int msgint = payload->toInt();                                       // convert it
  if (msgint >= report.mininterval && msgint <= report.maxinterval) {  // validate
    report.interval = msgint;                                          // set it
    return true;                                                       // report success
  } else if (msgint == 0) {                                            // failed to convert payload to a number
    *payload = F("requested interval NaN");                            // let them know
    return false;                                                      // send it
  } else {                                                             // the payload received was a number, but out of acceptable bounds
    *payload = F("requested interval invalid");                        // let them know
    return false;                                                      // send it
  }                                                                    // end payload validation
  return false;                                                        // theoretically, we should NEVER reach this, but just in case
}

bool set_manuallag(String* payload) {   // convert payload to integer, validate, and apply it if good
  int msgint = payload->toInt();        // convert it
  if (msgint >= 0 && msgint <= 1000) {  // validate
    delay(msgint);                      // execute
    return true;                        // report success
  } else if (msgint == 0) {             // failed to convert payload to a number
    *payload = F("delay NaN");          // let them know
    return false;                       // send it
  } else {                              // the payload received was a number, but out of acceptable bounds
    *payload = F("delay invalid");      // let them know
    return false;                       // send it
  }                                     // end payload validation
  return false;                         // theoretically, we should NEVER reach this, but just in case
}

bool set_offset(String* payload) {             // convert payload to float, validate, and apply it if good
  int msgint = payload->toInt();               // convert it
  if (msgint >= 0 && msgint <= 4096) {         // validate
    az.RAWOFFSET = 4096 - msgint;              // set it
    return true;                               // report success
  } else if (msgint == 0) {                    // failed to convert payload to a number
    *payload = F("requested offset NaN");      // let them know
    return false;                              // send it
  } else {                                     // the payload received was a number, but out of acceptable bounds
    *payload = F("requested offset invalid");  // let them know
    return false;                              // send it
  }                                            // end payload validation
  return false;                                // theoretically, we should NEVER reach this, but just in case
}

void cmd_setzero() {                         // set our current position as zero
  az.RAWOFFSET = 4096 - as5600.readAngle();  // set it
}















/*
  switch (format) {  // go ahead and interpret the payload for later use
    case 'B':
      if (payload.startsWith("T")) { boolinput = true; }
      if (payload.startsWith("F")) { boolinput = false; }
      break;
    case 'I':
      integerinput = payload.toInt();
      break;
    case 'F':
      floatinput = payload.toFloat();
      break;
    default:
      HC12.println(F("$ack#err#payloadformat$"));
      return;
  }
        case 'T':        // CT_Iint
          delay(input);  // manually lags the node by X msec; useful for trying to sync Azimuth to Data packets
          return;
        default:
          HC12.println(F("$ack#err#CMDtarget$"));
          return;
      }

              cmd_azimuthoffset(&floatinput);  // SSOF____
              return;
              report.msgcount = 0;  // SNC____ no payload needed
              HC12.println(F("$ack#suc#msgcountreset$"));
              return;
            case 'I':
              cmd_setreportinterval(integerinput);  // SNII____ set interval
              return;
            default:
              HC12.println("ack,err,SETNsubtarget");
              return;
          }
   
*/
