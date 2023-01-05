// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef LLVM_FUZZER_RANDOM_H
#define LLVM_FUZZER_RANDOM_H

#include <random>

namespace fuzzer {
class Random : public std::minstd_rand {
 public:
  Random(unsigned int seed) : std::minstd_rand(seed) {}
  result_type operator()() { return this->std::minstd_rand::operator()(); }
  size_t Rand() { return this->operator()(); }
  size_t RandBool() { return Rand() % 2; }
  size_t SkewTowardsLast(size_t n) {
    size_t T = this->operator()(n * n);
    size_t Res = sqrt(T);
    return Res;
  }
  size_t operator()(size_t n) { return n ? Rand() % n : 0; }
  intptr_t operator()(intptr_t From, intptr_t To) {
    assert(From < To);
    intptr_t RangeSize = To - From + 1;
    return operator()(RangeSize) + From;
  }
};

}  // namespace fuzzer

#endif  // LLVM_FUZZER_RANDOM_H
