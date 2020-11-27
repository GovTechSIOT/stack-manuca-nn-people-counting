/*******************************************************************************************************
 * Copyright (c) 2018-2019 Government Technology Agency of Singapore (GovTech)
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
#ifndef TRACE_MACRO_H
#define TRACE_MACRO_H

#define TRACE \
\
\
/* Sensor Thread*/ \
X(POLL_RATE_UPDATE, "poll_rate_updated")
/* --------------------------------------- */
#define X(code, value) code,
enum Trace : size_t
{
    TRACE
};
#undef X

#define X(code, value) value,
static char const *trace_name[50] =
{
    TRACE
};
#undef X
/* --------------------------------------- */

#endif  // TRACE_MACRO_H