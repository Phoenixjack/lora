#include "HardwareSerial.h"
#include "Arduino.h"
#include <stdint.h>
#include "data_packet.h"

class PhoenixJack_LORA {
public:
  enum ERROR_LEVEL {
    DATA_RCVD,
    ERROR_NO_DATA,
    ERROR_BUFFER_OVERFLOW,
    ERROR_EOM_RCVD,
    ERROR_TIMED_OUT,
    ERROR_END_OF_LIST
  };
private:
  HardwareSerial *_stream = nullptr;
  uint8_t _config_pin = 12;
  bool _config_mode = false;
  bool _ready = false;
  uint8_t _channel = 75;
  uint8_t _power_setting = 8;
  const uint8_t _max_buffer_size = 61;
  struct _buffer {
    uint8_t index = 0;
    char buffer[61];
  } _tx, _rx;
  bool _is_EOM(char c);
  ERROR_LEVEL _check();
public:
  explicit PhoenixJack_LORA(uint8_t set_pin, HardwareSerial *io_stream);
  void enter_config_mode();
  void exit_config_mode();
  bool initialize();
};

PhoenixJack_LORA::PhoenixJack_LORA(uint8_t set_pin, HardwareSerial *io_stream) {
  _config_pin = set_pin;
  _stream = io_stream;
};
PhoenixJack_LORA::ERROR_LEVEL PhoenixJack_LORA::_check() {
  ERROR_LEVEL _return_value;
  if (_stream->available()) {
    char inByte = _stream->read();
    if (_is_EOM(inByte)) {
      _rx.buffer[_rx.index] = (char)0;
      _return_value = ERROR_EOM_RCVD;
    } else {
      _rx.buffer[_rx.index] = inByte;
      _rx.index++;
      if (_rx.index < _max_buffer_size) {
        _return_value = DATA_RCVD;
      } else {
        _return_value = ERROR_BUFFER_OVERFLOW;
      }
    }
  } else {
    _return_value = ERROR_NO_DATA;
  }
  return _return_value;
};
bool PhoenixJack_LORA::initialize() {
  _stream->begin(9600);
  pinMode(_config_pin, OUTPUT);
  enter_config_mode();
  _rx.index = 0;
  _ready = false;
  bool _not_timed_out = true;
  _rx.buffer[0] = '1';
  _rx.buffer[1] = '1';
  unsigned long _timeout = millis() + 2500;
  _stream->print(F("AT"));
  while (_not_timed_out && !_ready) {
    //if (millis() > _timeout) { _not_timed_out = false; }
    if (_check() == DATA_RCVD) {
      if (_rx.buffer[0] == 'O' && _rx.buffer[1] == 'K') { _ready = true; }
      Serial.println(_rx.buffer[_rx.index]);
    }
  }
  _rx.buffer[2] = (char)0;
  Serial.println(_rx.buffer);
  exit_config_mode();
  return _ready;
};
void PhoenixJack_LORA::enter_config_mode() {
  digitalWrite(_config_pin, LOW);
  _config_mode = true;
};
void PhoenixJack_LORA::exit_config_mode() {
  digitalWrite(_config_pin, HIGH);
  _config_mode = false;
};
bool PhoenixJack_LORA::_is_EOM(char c) {
  if (c == (char)10 || c == (char)13) {
    _stream->flush();
    return true;
  } else {
    return false;
  }
};