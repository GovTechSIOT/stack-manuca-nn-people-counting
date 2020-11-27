/*******************************************************************************************************
 * Copyright (c) 2018-2020 Government Technology Agency of Singapore (GovTech)
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 *
 * You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied.
 *
 * See the License for the specific language governing permissions and limitations under the License.
 *******************************************************************************************************/
#ifndef COMMUNICATIONS_NETWORK_H
#define COMMUNICATIONS_NETWORK_H

#include <string>
#include <unordered_set>
#include "mbed.h"

#undef USE_WIFI
#if defined(MBED_CONF_APP_USE_WIFI) && (MBED_CONF_APP_USE_WIFI == 1)
    #define USE_WIFI
#endif  // USE_WIFI

/* Network Connection */
bool ConfigNetworkInterface(NetworkInterface*& network);

#endif  // COMMUNICATIONS_NETWORK_H