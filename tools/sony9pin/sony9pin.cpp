/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the same directory.
 */

#include <QCoreApplication>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QThread>
#include <iomanip>
#include <iostream>
#include <iterator>
using namespace std;

// #define SONY9PINREMOTE_DEBUGLOG_ENABLE
#include "Sony9PinRemote/Sony9PinRemote.h"
Sony9PinRemote::Controller deck;

QSerialPort serialPort;

void commands(const char* const prefix = "") {
  std::cerr << prefix << "Commands:\n"
    << prefix << "-: interactive (1-char command then enter)\n"
    << prefix << "e: eject\n"
    << prefix << "f: fast_forward\n"
    << prefix << "p: play\n"
    << prefix << "r: rewind\n"
    << prefix << "s: stop\n"
    << prefix << "0: status\n"
    << prefix << "1: type\n"
    << prefix << "2: timer1\n"
    << prefix << "3: timer2\n"
    << prefix << "4: ltc_tc_ub\n"
    << prefix << "5: vitc_tc_ub\n"
    ;
}

void help(const string& commandName) {
  std::cerr << "Usage: " << commandName << " <SerialPortName/SerialPortIndex> [command].\n";
  commands();
}

void usage(const string& commandName) {
  help(commandName);
  std::cerr << "Port names:\n";
  const auto portInfos = QSerialPortInfo::availablePorts();
  for (int i = 0; i < portInfos.size(); i++) {
    const auto& portInfo = portInfos[i];
    std::cerr << "  " << i << ": "
        << portInfo.portName().toStdString() << '\n';
  }
}

int setup(const QString& serialPortName) {
  // Config
  bool portNumberIsOk = false;
  const auto portNumber = serialPortName.toInt(&portNumberIsOk);
  if (portNumberIsOk) {
    const auto portInfos = QSerialPortInfo::availablePorts();
    if (portNumber >= portInfos.size()) {
      std::cerr << "Error: wrong port index.\n";
      return 1;
    }
    const auto& portInfo = portInfos[portNumber];
    serialPort.setPort(portInfo);
  } else {
    serialPort.setPortName(serialPortName);
  }
  serialPort.setBaudRate(Sony9PinSerial::BAUDRATE);
  serialPort.setParity(QSerialPort::OddParity);

  // Open
  if (true) {
    std::cerr << "Info: open device " << serialPort.portName().toStdString() << ".\n";
  }
  if (!serialPort.open(QIODevice::ReadWrite)) {
    std::cerr << "Error: open device fail.\n";
    return 1;
  }
  //QThread::msleep(2000);
  deck.attach(serialPort);
  if (true) {
    std::cerr << "Info: open device OK.\n";
  }

  return 0;
}

int status(){
  // Device status
  if (true) {
    std::cerr << "Info: get device status.\n";
  }
  deck.status_sense();
  if (!deck.parse_until(1000)) {
    std::cerr << "Error: get device status failed.\n";
    return 1;
  }
  deck.print_status();

  // Checks
  if (!deck.is_media_exist()) {
    std::cerr << "Error: there is no media.\n";
  }
  if (!deck.is_remote_enabled()) {
    std::cerr << "Error: remote control is disabled.\n";
  }
  if (!deck.is_disk_available()) {
    std::cerr << "Error: removable media is not available.\n";
  }
  if (!deck.is_stopping()) {
    std::cerr << "Info: stop deck.\n";
    deck.stop();
    if (!deck.parse_until(1000)) {
      std::cerr << "Error: stop deck failed.\n";
      return 1;
    }
  }

  return 0;
}

 int type() {
  if (true) {
    std::cerr << "Info: get device type.\n";
  }
  deck.device_type_request();
  if (!deck.parse_until(1000)) {
    std::cerr << "Error: get device type failed.\n";
    return 1;
  }
  const auto device_type = deck.device_type();
  std::cerr << "Info: device type is 0x" << setw(4) << setfill('0') << hex << device_type;
  switch (device_type) {
    case Sony9PinDevice::BLACKMAGIC_HYPERDECK_STUDIO_MINI_NTSC: {
      std::cerr << " (Blackmagic Hyperdeck Studio Mini, NTSC)";
      break;
    }
    case Sony9PinDevice::BLACKMAGIC_HYPERDECK_STUDIO_MINI_PAL: {
      std::cerr << " (Blackmagic Hyperdeck Studio Mini, PAL)";
      break;
    }
    case Sony9PinDevice::BLACKMAGIC_HYPERDECK_STUDIO_MINI_24P: {
      std::cerr << " (Blackmagic Hyperdeck Studio Mini, NTSC)";
      break;
    }
  }
  std::cerr << ".\n";

  return 0;
}

int ready() {
  while (!deck.ready()) {
    if (true) {
      std::cout << "Info: deck is not ready, waiting." << std::endl;
    }
    if (deck.parse_until(1000)) {
      break;
    }
  }
  return 0;
}

int eject() {
  if (true) {
    std::cout << "Info: eject." << std::endl;
  }
  deck.eject();
  if (!deck.parse_until(1000)) {
    std::cerr << "Error: eject failed.\n";
    return 1;
  }

  if (!deck.ack()) {
    std::cout << "Info: eject issue.\n";
    deck.print_nak();
  }

  return 0;
}

int fast_forward() {
  if (true) {
    std::cout << "Info: fast_forward." << std::endl;
  }
  deck.fast_forward();
  if (!deck.parse_until(1000)) {
    std::cerr << "Error: fast_forward failed.\n";
    return 1;
  }

  if (!deck.ack()) {
    std::cout << "Info: fast_forward issue.\n";
    deck.print_nak();
  }

  return 0;
}

int play() {
  if (true) {
    std::cout << "Info: play." << std::endl;
  }
  deck.play();
  if (!deck.parse_until(1000)) {
    std::cerr << "Error: play failed.\n";
    return 1;
  }

  if (!deck.ack()) {
    std::cout << "Info: play issue.\n";
    deck.print_nak();
  }

  return 0;
}

int rewind() {
  if (true) {
    std::cout << "Info: rewind." << std::endl;
  }
  deck.rewind();
  if (!deck.parse_until(1000)) {
    std::cerr << "Error: rewind failed.\n";
    return 1;
  }

  if (!deck.ack()) {
    std::cout << "Info: rewind issue.\n";
    deck.print_nak();
  }

  return 0;
}

int stop() {
  if (true) {
    std::cout << "Info: stop." << std::endl;
  }
  deck.stop();
  if (!deck.parse_until(1000)) {
    std::cerr << "Error: stop failed.\n";
    return 1;
  }

  if (!deck.ack()) {
    std::cout << "Info: stop issue.\n";
    deck.print_nak();
  }

  return 0;
}

int timer1() {
  if (true) {
    std::cout << "Info: timer1." << std::endl;
  }
  deck.current_time_sense_timer1();
  if (!deck.parse_until(1000)) {
    std::cerr << "Error: timer1 failed.\n";
    return 1;
  }

  if (!deck.ack()) {
    std::cout << "Info: timer1 issue.\n";
    deck.print_nak();
  }

  deck.print_timecode();

  return 0;
}

int timer2() {
  if (true) {
    std::cout << "Info: timer2." << std::endl;
  }
  deck.current_time_sense_timer2();
  if (!deck.parse_until(1000)) {
    std::cerr << "Error: timer2 failed.\n";
    return 1;
  }

  if (!deck.ack()) {
    std::cout << "Info: timer2 issue.\n";
    deck.print_nak();
  }

  deck.print_timecode();

  return 0;
}

int ltc_tc_ub() {
  if (true) {
    std::cout << "Info: ltc_tc_ub." << std::endl;
  }
  deck.current_time_sense_ltc_tc_ub();
  if (!deck.parse_until(1000)) {
    std::cerr << "Error: ltc_tc_ub failed.\n";
    return 1;
  }

  if (!deck.ack()) {
    std::cout << "Info: ltc_tc_ub issue.\n";
    deck.print_nak();
  }

  deck.print_timecode_userbits();

  return 0;
}

int vitc_tc_ub() {
  if (true) {
    std::cout << "Info: vitc_tc_ub." << std::endl;
  }
  deck.current_time_sense_ltc_tc_ub();
  if (!deck.parse_until(1000)) {
    std::cerr << "Error: vitc_tc_ub failed.\n";
    return 1;
  }

  if (!deck.ack()) {
    std::cout << "Info: vitc_tc_ub issue.\n";
    deck.print_nak();
  }

  deck.print_timecode_userbits();

  return 0;
}

void interactive(bool& is_interactive) {
  is_interactive = true;
  cerr << "Info: interactive mode.\n";
  commands("Info: ");
}

int main(int argc, char* argv[]) {
  QCoreApplication coreApplication(argc, argv);
  const int argumentCount = QCoreApplication::arguments().size();
  const QStringList argumentList = QCoreApplication::arguments();

  if (argumentCount == 1) {
    usage(argumentList.at(0).toStdString());
    return 1;
  }
  const auto& serialPortName = argumentList.at(1);

  if (auto result = setup(serialPortName)) {
    return result;
  }

  int i = 2;
  auto is_interactive = false;
  if (argumentCount == 2) {
    interactive(is_interactive);
    i--;
  }
  while (i < argumentCount) {
    if (const auto result = ready()) {
      return result;
    }
    char value; 
    if (is_interactive) {
      cin.get(value);
      cin.ignore(1024, '\n');
    } else {
      const auto& argument = argumentList[i];
      value = argument[0].toLatin1();
    }
    switch (value) {
      case '-': {
        interactive(is_interactive);
        break;
      }
      case '0': {
        if (const auto result = status()) {
          return result;
        }
        break;
      }
      case '1': {
        if (const auto result = type()) {
          return result;
        }
        break;
      }
      case '2': {
        if (const auto result = timer1()) {
          return result;
        }
        break;
      }
      case '3': {
        if (const auto result = timer2()) {
          return result;
        }
        break;
      }
      case '4': {
        if (const auto result = ltc_tc_ub()) {
          return result;
        }
        break;
      }
      case '5': {
        if (const auto result = vitc_tc_ub()) {
          return result;
        }
        break;
      }
      case 'e': {
        if (const auto result = eject()) {
          return result;
        }
        break;
      }
      case 'f': {
        if (const auto result = fast_forward()) {
          return result;
        }
        break;
      }
      case 'p': {
        if (const auto result = play()) {
          return result;
        }
        break;
      }
      case 'r': {
        if (const auto result = rewind()) {
          return result;
        }
        break;
      }
      case 's': {
        if (const auto result = stop()) {
          return result;
        }
        break;
      }
      default: {
        std::cerr << "Error: unknown command " << value << ".\n ";
      }
    }

    if (!is_interactive) {
      i++;
    }
  }

  return 0;
}
