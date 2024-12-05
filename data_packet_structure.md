LORA PACKET STRUCTURE:
  SERVER SIDE:
    - ACKNOWLEDGE
        + BYTE 1    : START BYTE '$'
        + BYTE 2    : MESSAGE PACKET VERSION
        + BYTE 3    : SENDER ID
        + BYTE 4    : INTENDED RECIPIENT ID
        + BYTE 5-8  : TIMESTAMP (MILLIS OR EPOCH)
        + BYTE 9    : STOP BYTE (char)255
    - COMMAND
        + 

  NODE SIDE:
    - COMMON TO ALL PACKETS:
        + BYTE 1    : START BYTE '$'
        + BYTE 2    : MESSAGE PACKET VERSION
        + BYTE 3    : SENDER ID
        + BYTE xx   : STOP BYTE (char)255
    - STANDARD REPORT
        + BYTE 4    : INTENDED RECIPIENT ID
        + BYTE 5    : REGISTERS
        + BYTE 6-9  : TIMESTAMP (MILLIS OR EPOCH)
        + BYTE 10   : DATA TYPE
        + BYTE 11-14: DATA
        + BYTE 15-16: CRC BYTE
    - EXPEDITED REPORT
        + BYTE 4-7  : TIMESTAMP (MILLIS OR EPOCH)
        + BYTE 8    : DATA TYPE
        + BYTE 9-12 : DATA
        + BYTE 13-14: CRC BYTE
    - ERROR REPORT
    - DATA AVAILABLE / FORMAT
    - REPORT CONFIG
    - ACKNOWLEDGE/REPEATBACK COMMAND

