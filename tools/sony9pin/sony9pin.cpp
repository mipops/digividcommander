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

void options(const char* const prefix = "") {
  std::cerr << prefix << "Options:\n"
    << prefix << "-v: verbose mode\n"
    << prefix << "-h: show help\n"
    ;
}

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
  std::cerr << "Usage: " << commandName << " [option] <SerialPortName/SerialPortIndex> [command].\n";
  options();
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

void print_timecode(const Sony9PinRemote::TimeCode& tc)
{
  cerr << "TimeCode: "
       << setw(2) << setfill('0') << (unsigned int)tc.hour << ':'
       << setw(2) << setfill('0') << (unsigned int)tc.minute << ':'
       << setw(2) << setfill('0') << (unsigned int)tc.second << ';'
       << setw(2) << setfill('0') << (unsigned int)tc.frame << ' '
       << "CF: " << (unsigned int)tc.is_cf << ' '
       << "DF: " << (unsigned int)tc.is_df << '\n';
}

void print_userbits(const Sony9PinRemote::UserBits& ub)
{
  cerr << "UserBits: "
       << (char)ub.bytes[0] << ' '
       << (char)ub.bytes[1] << ' '
       << (char)ub.bytes[2] << ' '
       << (char)ub.bytes[3] << '\n';
}

void print_timecode_userbits(const Sony9PinRemote::TimeCodeAndUserBits& tcub)
{
    print_timecode(tcub.tc);
    print_userbits(tcub.ub);
}

int setup(const QString& serialPortName, bool verbose) {
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
  if (verbose) {
    std::cerr << "Info: open device " << serialPort.portName().toStdString() << ".\n";
  }
  if (!serialPort.open(QIODevice::ReadWrite)) {
    std::cerr << "Error: open device fail.\n";
    return 1;
  }
  //QThread::msleep(2000);
  deck.attach(serialPort);
  if (verbose) {
    std::cerr << "Info: open device OK.\n";
  }

  return 0;
}

int status(bool verbose){
  // Device status
  if (verbose) {
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

 int type(bool verbose) {
  if (verbose) {
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

int ready(bool verbose) {
  while (!deck.ready()) {
    if (verbose) {
      std::cout << "Info: deck is not ready, waiting." << std::endl;
    }
    if (deck.parse_until(1000)) {
      break;
    }
  }
  return 0;
}

int check_status_for_command()
{
  deck.status_sense();
  if (!deck.parse_until(1000)) {
    std::cerr << "Error: get device status failed.\n";
    return 1;
  }

  if (!deck.is_remote_enabled()) {
    std::cerr << "Error: The device is in local mode. Please switch to remote and try again.\n";
    return 1;
  }

  if (!deck.is_media_exist()) {
    std::cerr << "Error: The device does not contain a cassette. Please insert media and try again.\n";
    return 1;
  }

  return 0;
}

int eject(bool verbose) {
  if (auto result = check_status_for_command()) {
    return result;
  }

  if (verbose) {
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

int fast_forward(bool verbose) {
  if (auto result = check_status_for_command()) {
    return result;
  }

  if (verbose) {
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

int play(bool verbose) {
  if (auto result = check_status_for_command()) {
    return result;
  }

  if (verbose) {
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

int rewind(bool verbose) {
  if (auto result = check_status_for_command()) {
    return result;
  }

  if (verbose) {
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

int stop(bool verbose) {
  if (auto result = check_status_for_command()) {
    return result;
  }

  if (verbose) {
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

int timer1(bool verbose) {
  if (verbose) {
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

  print_timecode(deck.timecode());

  return 0;
}

int timer2(bool verbose) {
  if (verbose) {
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

  print_timecode(deck.timecode());

  return 0;
}

int ltc_tc_ub(bool verbose) {
  if (verbose) {
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

  print_timecode_userbits(deck.timecode_userbits());

  return 0;
}

int vitc_tc_ub(bool verbose) {
  if (verbose) {
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

  print_timecode_userbits(deck.timecode_userbits());

  return 0;
}

void interactive(bool& is_interactive) {
  is_interactive = true;
  cerr << "Info: interactive mode.\n";
  commands("Info: ");
}

int main(int argc, char* argv[]) {
  QCoreApplication coreApplication(argc, argv);
  QStringList argumentList = QCoreApplication::arguments();

  QString commandName;
  if (!argumentList.isEmpty())
    commandName = argumentList.takeFirst();

  bool verbose = false;
  while (!argumentList.isEmpty())
  {
    if (argumentList.first() == "-h") {
      usage(commandName.toStdString());
      return 0;
    }
    else if (argumentList.first() == "-v") {
        verbose = true;
        cerr << "Info: verbose mode.\n";
        argumentList.removeFirst();
    }
    else
        break;
  }

  if (argumentList.isEmpty()) {
    usage(commandName.toStdString());
    return 1;
  }

  const auto& serialPortName = argumentList.takeFirst();
  if (auto result = setup(serialPortName, verbose)) {
    return result;
  }

  auto is_interactive = false;
  if (argumentList.isEmpty()) {
    interactive(is_interactive);
  }
  while (!argumentList.isEmpty() || is_interactive) {
    if (const auto result = ready(verbose)) {
      return result;
    }
    char value; 
    if (is_interactive) {
      cin.get(value);
      cin.ignore(1024, '\n');
    } else {
      const auto& argument = argumentList.takeFirst();
      value = argument[0].toLatin1();
    }
    switch (value) {
      case '-': {
        interactive(is_interactive);
        break;
      }
      case '0': {
        if (const auto result = status(verbose)) {
          return result;
        }
        break;
      }
      case '1': {
        if (const auto result = type(verbose)) {
          return result;
        }
        break;
      }
      case '2': {
        if (const auto result = timer1(verbose)) {
          return result;
        }
        break;
      }
      case '3': {
        if (const auto result = timer2(verbose)) {
          return result;
        }
        break;
      }
      case '4': {
        if (const auto result = ltc_tc_ub(verbose)) {
          return result;
        }
        break;
      }
      case '5': {
        if (const auto result = vitc_tc_ub(verbose)) {
          return result;
        }
        break;
      }
      case 'e': {
        if (const auto result = eject(verbose)) {
          return result;
        }
        break;
      }
      case 'f': {
        if (const auto result = fast_forward(verbose)) {
          return result;
        }
        break;
      }
      case 'p': {
        if (const auto result = play(verbose)) {
          return result;
        }
        break;
      }
      case 'r': {
        if (const auto result = rewind(verbose)) {
          return result;
        }
        break;
      }
      case 's': {
        if (const auto result = stop(verbose)) {
          return result;
        }
        break;
      }
      default: {
        std::cerr << "Error: unknown command " << value << ".\n ";
      }
    }
  }

  return 0;
}
