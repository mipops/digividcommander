/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the same directory.
 */

#include <QCoreApplication>
#include <QRegularExpression>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDateTime>
#include <QThread>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>

using namespace std;

const char* version = "1.0";

// #define SONY9PINREMOTE_DEBUGLOG_ENABLE
#include "Sony9PinRemote/Sony9PinRemote.h"

bool operator!=(const sony9pin::TimeCode& first, const sony9pin::TimeCode& second) {
  return first.is_cf != second.is_cf ||
         first.is_df != second.is_df ||
         first.frame != second.frame ||
         first.second != second.second ||
         first.minute != second.minute ||
         first.hour != second.hour;
}

struct State {
  Sony9PinRemote::TimeCode tc;
  Sony9PinRemote::Status st;
};

Sony9PinRemote::Controller deck;
QSerialPort serialPort;
State lastState;

extern map<uint16_t, pair<string, vector<string>>> devices;

void options(const char* const prefix = "") {
  std::cerr << prefix << "Options:\n"
    << prefix << "-c, --continuous: report deck state until stop bit is set\n"
    << prefix << "-v, --verbose: verbose mode\n"
    << prefix << "-V, --version: show version\n"
    << prefix << "-h, --help: show help\n"
    ;
}

void commands(const char* const prefix = "") {
  std::cerr << prefix << "Commands:\n"
    << prefix << "-: interactive (1-char command then enter)\n"
    << prefix << "e: eject\n"
    << prefix << "f: fast_forward\n"
    << prefix << "x: frame_step_forward\n"
    << prefix << "w: frame_step_reverse\n"
    << prefix << "p: play\n"
    << prefix << "r: rewind\n"
    << prefix << "s: stop\n"
    << prefix << "c <timecode in HH:mm:ss:ff format>: cue_up_with_data\n"
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

void print_timecode_userbits(bool print_userbits)
{
  Sony9PinRemote::TimeCode tc = deck.timecode();
  cerr << "TimeCode: " << dec
       << setw(2) << setfill('0') << (unsigned int)tc.hour << ':'
       << setw(2) << setfill('0') << (unsigned int)tc.minute << ':'
       << setw(2) << setfill('0') << (unsigned int)tc.second << ';'
       << setw(2) << setfill('0') << (unsigned int)tc.frame << ' '
       << "CF: " << (unsigned int)tc.is_cf << ' '
       << "DF: " << (unsigned int)tc.is_df
       << resetiosflags(std::ios::dec);

  if (print_userbits) {
    Sony9PinRemote::UserBits ub = deck.userbits();
    cerr << " UB: " << hex << uppercase
         << setw(2) << setfill('0') << (unsigned int)ub.bytes[3] << ':'
         << setw(2) << setfill('0') << (unsigned int)ub.bytes[2] << ':'
         << setw(2) << setfill('0') << (unsigned int)ub.bytes[1] << ':'
         << setw(2) << setfill('0') << (unsigned int)ub.bytes[0]
         << resetiosflags(std::ios::hex|std::ios::uppercase);
  }

  cerr << '\n';
}

bool test_ack()
{
  if (!deck.ack() && (deck.is_nak_unknown_command() ||
                      deck.is_nak_checksum_error() ||
                      deck.is_nak_parity_error() ||
                      deck.is_nak_buffer_overrun() ||
                      deck.is_nak_framing_error() ||
                      deck.is_nak_timeout()))
    return false;

  return true;
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
  std::cerr << "Info: device_type=0x" << hex << setw(4) << setfill('0') << device_type << resetiosflags(std::ios::hex);
  std::string device_make;
  std::string device_model;
  switch (device_type) {
    case Sony9PinDevice::BLACKMAGIC_HYPERDECK_STUDIO_MINI_NTSC: {
      device_make = "Blackmagic";
      device_model = "Hyperdeck Studio Mini, NTSC";
      break;
    }
    case Sony9PinDevice::BLACKMAGIC_HYPERDECK_STUDIO_MINI_PAL: {
      device_make = "Blackmagic";
      device_model = "Hyperdeck Studio Mini, PAL";
      break;
    }
    case Sony9PinDevice::BLACKMAGIC_HYPERDECK_STUDIO_MINI_24P: {
      device_make = "Blackmagic";
      device_model = "Hyperdeck Studio Mini, 24P";
      break;
    default:
      if (devices.find(device_type) != devices.end())
      {
        device_make = devices[device_type].first;
        for (auto model : devices[device_type].second)
        {
            if (!device_model.empty())
              device_model += ", ";
            device_model += model;
        }
      }
    }
  }
  if (!device_make.empty())
    std::cerr << ", device_make=\"" << device_make << "\"";
  if (!device_model.empty())
    std::cerr << ", device_model=\"" << device_model << "\"";
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

  if (!test_ack()) {
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

  if (!test_ack()) {
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

  if (!test_ack()) {
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

  if (!test_ack()) {
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

  if (!test_ack()) {
    std::cout << "Info: stop issue.\n";
    deck.print_nak();
  }

  return 0;
}

int frame_step_forward(bool verbose) {
  if (auto result = check_status_for_command()) {
    return result;
  }

  if (verbose) {
    std::cout << "Info: frame_step_forward." << std::endl;
  }
  deck.frame_step_forward();
  if (!deck.parse_until(1000)) {
    std::cerr << "Error: frame_step_forward failed.\n";
    return 1;
  }

  if (!deck.ack()) {
    std::cout << "Info: frame_step_forward issue.\n";
    deck.print_nak();
  }

  return 0;
}

int cue_up_with_data(uint8_t hh, uint8_t mm, uint8_t ss, uint8_t ff, bool verbose)
{
  if (auto result = check_status_for_command()) {
    return result;
  }

  if (verbose) {
    std::cout << "Info: cue_up_with_data." << std::endl;
  }
  deck.cue_up_with_data(hh + 6 * (hh / 10), mm + 6 * (mm / 10), ss + 6 * (ss / 10), ff + 6 * (ff / 10));
  if (!deck.parse_until(1000)) {
    std::cerr << "Error: cue_up_with_data failed.\n";
    return 1;
  }

  if (!deck.ack()) {
    std::cout << "Info: cue_up_with_data issue.\n";
    deck.print_nak();
  }

  return 0;
}

int frame_step_reverse(bool verbose) {
  if (auto result = check_status_for_command()) {
    return result;
  }

  if (verbose) {
    std::cout << "Info: frame_step_reverse." << std::endl;
  }
  deck.frame_step_reverse();
  if (!deck.parse_until(1000)) {
    std::cerr << "Error: frame_step_reverse failed.\n";
    return 1;
  }

  if (!deck.ack()) {
    std::cout << "Info: frame_step_reverse issue.\n";
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

  if (!test_ack()) {
    std::cout << "Info: timer1 issue.\n";
    deck.print_nak();
  }

  print_timecode_userbits(false);

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

  if (!test_ack()) {
    std::cout << "Info: timer2 issue.\n";
    deck.print_nak();
  }

  print_timecode_userbits(false);

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

  if (!test_ack()) {
    std::cout << "Info: ltc_tc_ub issue.\n";
    deck.print_nak();
  }

  print_timecode_userbits(true);

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

  if (!test_ack()) {
    std::cout << "Info: vitc_tc_ub issue.\n";
    deck.print_nak();
  }

  print_timecode_userbits(true);

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

  bool verbose = false, continuous = false;
  while (!argumentList.isEmpty())
  {
    if (argumentList.first() == "--help" || argumentList.first() == "-h") {
      usage(commandName.toStdString());
      return 0;
    }
    else if (argumentList.first() == "--version" || argumentList.first() == "-V") {
      cerr << "sony9pin v" << version << " by MIPoPS\n";
      return 0;
    }
    else if (argumentList.first() == "--verbose" || argumentList.first() == "-v") {
        verbose = true;
        cerr << "Info: verbose mode.\n";
        argumentList.removeFirst();
    }
    else if (argumentList.first() == "--continuous" || argumentList.first() == "-c") {
        continuous = true;
        cerr << "Info: continuous mode.\n";
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
  if (!continuous && argumentList.isEmpty()) {
    interactive(is_interactive);
  }
  while (!argumentList.isEmpty() || is_interactive) {
    if (const auto result = ready(verbose)) {
      return result;
    }
    char value;
    string remains;
    if (is_interactive) {
      cin.get(value);
      getline(cin, remains);
    } else {
      const auto& argument = argumentList.takeFirst();
      value = argument[0].toLatin1();
    }
    switch (value) {
      case '-': {
        if (!continuous)
            interactive(is_interactive);
          else
            cerr << "Error: interactive input unavaiable in continuous mode.\n";
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
      case 'x': {
        if (const auto result = frame_step_forward(verbose)) {
          return result;
        }
        break;
      }
      case 'w': {
        if (const auto result = frame_step_reverse(verbose)) {
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
      case 'c': {
        QString param;
        if (is_interactive) {
          param = QString().fromStdString(remains).trimmed();
        } else if (!argumentList.isEmpty()) {
          param = argumentList.takeFirst();
        }

        QStringList tc = param.split(QRegularExpression("[:;]"));
        if (tc.size() != 4) {
          cerr << "Error: invalid timecode " << param.toStdString() << ".\n";
          return 1;
        }

        bool ok = false;
        uint8_t hh = tc[0].toUShort(&ok);
        if (!ok) {
          cerr << "Error: invalid timecode " << param.toStdString() << ".\n";
          return 1;
        }

        uint8_t mm = tc[1].toUShort(&ok);
        if (!ok) {
          cerr << "Error: invalid timecode " << param.toStdString() << ".\n";
          return 1;
        }

        uint8_t ss = tc[2].toUShort(&ok);
        if (!ok) {
          cerr << "Error: invalid timecode " << param.toStdString() << ".\n";
          return 1;
        }

        uint8_t ff = tc[3].toUShort(&ok);
        if (!ok) {
          cerr << "Error: invalid timecode " << param.toStdString() << ".\n";
          return 1;
        }

        if (const auto result = cue_up_with_data(hh, mm, ss, ff, verbose)) {
          return result;
        }
        break;
      }
      default: {
        std::cerr << "Error: unknown command " << value << ".\n ";
      }
    }
  }

  if (continuous) {
    bool first=true;
    bool stop = false;
    while (!stop) {
      State state;
      bool print = false;

      deck.status_sense();
      if (!deck.parse_until(1000))
        std::cerr << "Error: parse failed.\n";

      if (!test_ack()) {
        std::cout << "Info: parse issue.\n";
        deck.print_nak();
      }
      state.st = deck.status();

      deck.current_time_sense_timer1();
      if (!deck.parse_until(1000))
        std::cerr << "Error: parse failed.\n";

      if (!test_ack()) {
        std::cout << "Info: parse issue.\n";
        deck.print_nak();
      }
      state.tc = deck.timecode();

      std::stringstream ss;
      ss << dec << ' '
         << setw(2) << setfill('0') << (unsigned int)state.tc.hour << ':'
         << setw(2) << setfill('0') << (unsigned int)state.tc.minute << ':'
         << setw(2) << setfill('0') << (unsigned int)state.tc.second << ';'
         << setw(2) << setfill('0') << (unsigned int)state.tc.frame
         << resetiosflags(std::ios::dec);

      if (first || state.tc != lastState.tc) {
        print = true;
      }

      if (first || state.st.b_cassette_out != lastState.st.b_cassette_out) {
        ss << " cassette_out=" << (unsigned int)state.st.b_cassette_out;
        print = true;
      }

      if (first || state.st.b_servo_ref_missing != lastState.st.b_servo_ref_missing) {
        ss << " servo_ref_missing=" << (unsigned int)state.st.b_servo_ref_missing;
        print = true;
      }

      if (first || state.st.b_local != lastState.st.b_local) {
        ss << " local=" << (unsigned int)state.st.b_local;
        print = true;
      }

      if (first || state.st.b_standby != lastState.st.b_standby) {
        ss << " standby=" << (unsigned int)state.st.b_standby;
        print = true;
      }

      if (first || state.st.b_stop != lastState.st.b_stop) {
        ss << " stop=" << (unsigned int)state.st.b_stop;
        print = true;

        if (state.st.b_stop)
          stop = true;
      }

      if (first || state.st.b_eject != lastState.st.b_eject) {
        ss << " eject=" << (unsigned int)state.st.b_eject;
        print = true;
      }

      if (first || state.st.b_rewind != lastState.st.b_rewind) {
        ss << " rewind=" << (unsigned int)state.st.b_rewind;
        print = true;
      }

      if (first || state.st.b_forward != lastState.st.b_forward) {
        ss << " forward=" << (unsigned int)state.st.b_forward;
        print = true;
      }

      if (first || state.st.b_record != lastState.st.b_record) {
        ss << " record=" << (unsigned int)state.st.b_record;
        print = true;
      }

      if (first || state.st.b_play != lastState.st.b_play) {
        ss << " play=" << (unsigned int)state.st.b_play;
        print = true;
      }

      if (first || state.st.b_servo_lock != lastState.st.b_servo_lock) {
        ss << " servo_lock=" << (unsigned int)state.st.b_servo_lock;
        print = true;
      }

      if (first || state.st.b_tso_mode != lastState.st.b_tso_mode) {
        ss << " tso_mode=" << (unsigned int)state.st.b_tso_mode;
        print = true;
      }

      if (first || state.st.b_shuttle != lastState.st.b_shuttle) {
        ss << " shuttle=" << (unsigned int)state.st.b_shuttle;
        print = true;
      }

      if (first || state.st.b_jog != lastState.st.b_jog) {
        ss << " jog=" << (unsigned int)state.st.b_jog;
        print = true;
      }

      if (first || state.st.b_var != lastState.st.b_var) {
        ss << " var=" << (unsigned int)state.st.b_var;
        print = true;
      }

      if (first || state.st.b_direction != lastState.st.b_direction) {
        ss << " direction=" << (unsigned int)state.st.b_direction;
        print = true;
      }

      if (first || state.st.b_still != lastState.st.b_still) {
        ss << " still=" << (unsigned int)state.st.b_still;
        print = true;
      }

      if (first || state.st.b_cue_up != lastState.st.b_cue_up) {
        ss << " cue_up=" << (unsigned int)state.st.b_cue_up;
        print = true;
      }

      if (first || state.st.b_lamp_still != lastState.st.b_lamp_still) {
        ss << " lamp_still=" << (unsigned int)state.st.b_lamp_still;
        print = true;
      }

      if (first || state.st.b_lamp_fwd != lastState.st.b_lamp_fwd) {
        ss << " lamp_fwd=" << (unsigned int)state.st.b_lamp_fwd;
        print = true;
      }

      if (first || state.st.b_lamp_rev != lastState.st.b_lamp_rev) {
        ss << " lamp_rev=" << (unsigned int)state.st.b_lamp_rev;
        print = true;
      }

      if (first || state.st.b_near_eot != lastState.st.b_near_eot) {
        ss << " near_eot=" << (unsigned int)state.st.b_near_eot;
        print = true;
      }

      if (first || state.st.b_eot != lastState.st.b_eot) {
        ss << " eot=" << (unsigned int)state.st.b_eot;
        print = true;
      }

      if (first || state.st.b_cf_lock != lastState.st.b_cf_lock) {
        ss << " cf_lock=" << (unsigned int)state.st.b_cf_lock;
        print = true;
      }

      if (first || state.st.b_svo_alarm != lastState.st.b_svo_alarm) {
        ss << " svo_alarm=" << (unsigned int)state.st.b_svo_alarm;
        print = true;
      }

      if (first || state.st.b_sys_alarm != lastState.st.b_sys_alarm) {
        ss << " sys_alarm=" << (unsigned int)state.st.b_sys_alarm;
        print = true;
      }

      if (first || state.st.b_rec_inhib != lastState.st.b_rec_inhib) {
        ss << " rec_inhib=" << (unsigned int)state.st.b_rec_inhib;
        print = true;
      }

      if (print) {
        cout << QDateTime::currentDateTime().toString(Qt::ISODateWithMs).toStdString() << ss.str() << '\n';
        lastState=state;
      }
      first=false;
    }
  }

  return 0;
}
