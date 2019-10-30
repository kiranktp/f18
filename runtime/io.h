// Copyright (c) 2019, NVIDIA CORPORATION.  All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Defines API between compiled code and I/O runtime library.

#ifndef FORTRAN_RUNTIME_IO_H_
#define FORTRAN_RUNTIME_IO_H_

#include "entry-names.h"
#include <cinttypes>
#include <cstddef>

namespace Fortran::runtime {
class Descriptor;
class NamelistGroup;
};

namespace Fortran::runtime::IO {

class IOStatementState;
using Cookie = IOStatementState *;
using ExternalUnit = int;
using AsynchronousId = int;
static constexpr ExternalUnit DefaultUnit{-1};  // READ(*), WRITE(*), PRINT

extern "C" {

#define IONAME(n) RTNAME(IO_##n)

// These function initiate data transfer statements (READ, WRITE, PRINT).
// Example: PRINT *, 666 is implemented as the series of calls:
//   Cookie cookie{BeginExternalListOutput(DefaultUnit)};
//   OutputInteger64(cookie, 666);
//   EndIOStatement(cookie);

// Internal I/O initiation
// Internal I/O can loan the runtime library an optional block of memory
// in which to maintain state across the calls that implement the transfer;
// use of these blocks can reduce the need for dynamic memory allocation
// &/or thread-local storage.
Cookie IONAME(BeginInternalListOutput)(
  char *internal, std::size_t bytes, int characterKind = 1,
  void **scratchArea = nullptr, std::size_t scratchBytes = 0);
Cookie IONAME(BeginInternalListInput)(
  char *internal, std::size_t bytes, int characterKind = 1,
  void **scratchArea = nullptr, std::size_t scratchBytes = 0);
Cookie IONAME(BeginInternalFormattedOutput)(
  char *internal, std::size_t bytes,
  const char *format, std::size_t formatBytes,
  int characterKind = 1,
  void **scratchArea = nullptr, std::size_t scratchBytes = 0);
Cookie IONAME(BeginInternalFormattedInput)(
  char *internal, std::size_t bytes,
  const char *format, std::size_t formatBytes,
  int characterKind = 1,
  void **scratchArea = nullptr, std::size_t scratchBytes = 0);

// External synchronous I/O initiation
Cookie IONAME(BeginExternalListOutput)(ExternalUnit);
Cookie IONAME(BeginExternalListInput)(ExternalUnit);
Cookie IONAME(BeginExternalFormattedOutput)(ExternalUnit,
                                           const char *format, std::size_t);
Cookie IONAME(BeginExternalFormattedInput)(ExternalUnit,
                                           const char *format, std::size_t);
Cookie IONAME(BeginUnformattedOutput)(ExternalUnit);
Cookie IONAME(BeginUnformattedInput)(ExternalUnit);
Cookie IONAME(BeginNamelistOutput)(ExternalUnit, const NamelistGroup &);
Cookie IONAME(BeginNamelistInput)(ExternalUnit, const NamelistGroup &);

// Asynchronous I/O is supported (at most) for unformatted direct access
// block transfers.
AsynchronousID IONAME(BeginAsynchronousOutput)(ExternalUnit, std::int64_t REC, const char *, std::size_t);
AsynchronousID IONAME(BeginAsynchronousInput)(ExternalUnit, std::int64_t REC, char *, std::size_t);
Cookie IONAME(WaitForAsynchronousIO)(ExternalUnit, AsynchronousID);  // WAIT

// Other I/O statements
// TODO: OPEN & INQUIRE
Cookie IONAME(BeginClose)(ExternalUnit);
Cookie IONAME(BeginFlush)(ExternalUnit);
Cookie IONAME(BeginBackspace)(ExternalUnit);
Cookie IONAME(BeginEndfile)(ExternalUnit);
Cookie IONAME(BeginRewind)(ExternalUnit);

// Control list options
void IONAME(SetADVANCE)(Cookie, const char *, std::size_t);
void IONAME(SetBLANK)(Cookie, const char *, std::size_t);
void IONAME(SetDECIMAL)(Cookie, const char *, std::size_t);
void IONAME(SetDELIM)(Cookie, const char *, std::size_t);
void IONAME(SetPAD)(Cookie, const char *, std::size_t);
void IONAME(SetPOS)(Cookie, std::int64_t);
void IONAME(SetREC)(Cookie, std::int64_t);
void IONAME(SetROUND)(Cookie, const char *, std::size_t);
void IONAME(SetSIGN)(Cookie, const char *, std::size_t);

// Data item transfer for modes other than namelist.
// Any item can be transferred by means of a descriptor; unformatted
// transfers to/from contiguous blocks can avoid the descriptor; and there
// are specializations for the common scalar types.
void IONAME(OutputDescriptor)(Cookie, const Descriptor &);
void IONAME(InputDescriptor)(Cookie, const Descriptor &);
void IONAME(OutputUnformattedBlock)(Cookie, const char *, std::size_t);
void IONAME(InputUnformattedBlock)(Cookie, char *, std::size_t);
void IONAME(OutputInteger64)(Cookie, std::int64_t);
std::int64_t IONAME(InputInteger64)(Cookie);
void IONAME(OutputReal64)(Cookie, double);
double IONAME(InputReal64)(Cookie);
void IONAME(OutputASCII)(Cookie, const char *, std::size_t);
void IONAME(InputASCII)(Cookie, char *, std::size_t);

// Result extraction after data transfers are complete.
void IONAME(GetIOMSG)(Cookie, char *, std::size_t);  // IOMSG=
void IONAME(GetSTATUS)(Cookie, char *, std::size_t);  // STATUS=
int IONAME(GetIOSTAT)(Cookie);  // IOSTAT=
std::size_t IONAME(GetSIZE)(Cookie);  // SIZE=

bool IONAME(IsEND)(Cookie);
bool IONAME(IsERR)(Cookie);
bool IONAME(IsEOR)(Cookie);

// The cookie value must not be used after calling this.
void IONAME(EndIOStatement)(Cookie);
};
}
#endif
