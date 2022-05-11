/*  Copyright (c) MIPoPS. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-3-Clause license that can
 *  be found in the LICENSE.txt file in the same directory.
 */


#include <utility>
#include <string>
#include <map>

std::map<uint16_t, std::pair<std::string, std::string>> devices = {
  { 0x20a1, { "SONY", "SRW-5500"} },
  { 0x8017, { "SONY", "DSR-1500A" } },
  { 0xb000, { "SONY", "DVW-A500" } },
  { 0xb070, { "SONY", "J-3" } }
};
