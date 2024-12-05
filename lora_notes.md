# HC-12 info

*** max apparent data size = 60 characters ***
## REFERENCES:
  [User Manual](https://www.elecrow.com/download/HC-12.pdf)
  [How-To](https://howtomechatronics.com/tutorials/arduino/arduino-and-hc-12-long-range-wireless-communication-module/#google_vignette)

## QUICK NOTES:
  - Max data buffer: 60 characters
  - Power Range: -1dBm/794 uW  to 20 dBm/100mW
  - Freq Range: 433.4 MHz to 473.0 MHz
  - Speed Range: 1200 bps to 115200 bps

## COMMAND LIST:
  - AT
  - AT+Bxxxx
  - AT+Cxxx
  - AT+Px
  - AT+FUx
  - AT+Ry
  - AT+RX
  - AT+Uxxx
  - AT+V
  - AT+SLEEP
  - AT+DEFAULT

## RF POWER LEVEL NOTES:
  - dBm Equivalence:
    o 0dBm = 1mW (Bluetooth standard power)
    o -50dBm = 10nW
    o -60dBm = 1nW
    o -80dBm = 10pW
    o -100dBm = 0.1pW
    o -127.5dBm = 0.178fW (standard GPS signal)
  - Generally, every time the receiving sensitivity is reduced by 6dB, the communication distance will be reduced by half

## COMMAND DETAILS:
  - AT : Test command
    o EXAMPLE: 
      + Send:     “AT”
      + Response: “OK”
  - AT+Bxxxx : Change the serial port baud rate.
    o OPTIONS: 
      + 1200 bps
      + 2400 bps
      + 4800 bps
      + 9600 bps [DEFAULT]
      + 19200 bps
      + 38400 bps
      + 57600 bps
      + 115200 bps
    o EXAMPLE: 
      + Send:     “AT+B19200”
      + Response: “OK+B19200”
  - AT+Cxxxx : Change wireless communication channel, from 001 to 100
    o OPTIONS: 
      + Channel 001 = 433.4 MHz  [DEFAULT]
      + ... 400KHz increments
      + Channel 100 = 473.0 MHz
    o EXAMPLE: 
      + Send:     “AT+C006”
      + Response: “OK+C006”
  - AT+Px : Set transmitting power of module, x is optional from 1 to 8
    o OPTIONS: 
      + 1: -1 dBm / 794 uW
      + 2: 2 dBm  / 1.6 mW
      + 3: 5 dBm  / 3.2mW
      + 4: 8 dBm  / 6 mW
      + 5: 11 dBm / 13 mW
      + 6: 14 dBm / 25 mW
      + 7: 17 dBm / 50 mW
      + 8: 20 dBm / 100 mW  [DEFAULT]
    o EXAMPLE: 
      + Send:     “AT+P6”
      + Response: “OK+P6”
  - AT+FUx : Change serial port transparent transmission mode
    o OPTIONS: 
      + FU1 - relative power saving mode
        - 3.6mA idle current
        - 8 serial baud options
        - SINGLE over the air baud rate: 250Kbps
        - Transmission delay 15-25msec
      + FU2 - power saving mode
        - 80uA idle current
        - Serial baud options: 1.2, 2.4, or 4.8Kbps
        - OTA baud rate 250Kbps
        - Transmission delay 500msec
      + FU3 - default
        - 16mA idle current
        - Transmission delay 4-80msec
  - AT+Ry : Obtain single parameter of module, y is any letter among B, C, F and P, 
    o OPTIONS: 
      + B: baud rate
      + C: communication channel
      + F: serial port transparent transmission mode
      + P: transmitting power
    o EXAMPLE: 
      + Send:     “AT+RB”
      + Response: “OK+B9600”
  - AT+RX : Obtain ALL parameters of module.
  - AT+Uxyz : Set data bits, check bit and stop bit of serial port communication.
    o OPTIONS: 
      + x: Number of data bits
      + y: Check bit
        - N: no check
        - O: odd check
        - E: even check
      + z: Number of stop bits
        - 1: 1 stop bit
        - 2: 2 stop bits
        - 3: 1.5 stop bits
    o EXAMPLE: 
      + Send:     “AT+U8O1”
      + Response: “OK+U8O1”
  - AT+V : Inquire firmware version information of module.
  - AT+SLEEP : Enter sleep mode
    o Working current: 22uA
    o Enter "AT" to exit sleep mode
  - AT+DEFAULT : Set serial port baud rate, communication channel, and serial port transparent transmission mode to be default value


