// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef LLVM_FUZZER_EXT_FUNCTIONS_H
#define LLVM_FUZZER_EXT_FUNCTIONS_H

#include <stddef.h>
#include <stdint.h>

namespace fuzzer {

struct ExternalFunctions {
  // Initialize function pointers. Functions that are not available will be set
  // to nullptr.  Do not call this constructor  before ``main()`` has been
  // entered.
  ExternalFunctions();

#define EXT_FUNC(NAME, RETURN_TYPE, FUNC_SIG, WARN)                            \
  RETURN_TYPE(*NAME) FUNC_SIG = nullptr

#include "FuzzerExtFunctions.def"

#undef EXT_FUNC
};
} // namespace fuzzer

#endif
