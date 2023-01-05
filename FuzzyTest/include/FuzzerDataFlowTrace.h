// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef LLVM_FUZZER_DATA_FLOW_TRACE
#define LLVM_FUZZER_DATA_FLOW_TRACE

#include "FuzzerDefs.h"
#include "FuzzerIO.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>

namespace fuzzer {

int CollectDataFlow(const std::string &DFTBinary, const std::string &DirPath,
                    const Vector<SizedFile> &CorporaFiles);

class BlockCoverage {
 public:
  bool AppendCoverage(std::istream &IN);
  bool AppendCoverage(const std::string &S);

  size_t NumCoveredFunctions() const { return Functions.size(); }

  uint32_t GetCounter(size_t FunctionId, size_t BasicBlockId) {
    auto It = Functions.find(FunctionId);
    if (It == Functions.end()) return 0;
    const auto &Counters = It->second;
    if (BasicBlockId < Counters.size())
      return Counters[BasicBlockId];
    return 0;
  }

  uint32_t GetNumberOfBlocks(size_t FunctionId) {
    auto It = Functions.find(FunctionId);
    if (It == Functions.end()) return 0;
    const auto &Counters = It->second;
    return Counters.size();
  }

  uint32_t GetNumberOfCoveredBlocks(size_t FunctionId) {
    auto It = Functions.find(FunctionId);
    if (It == Functions.end()) return 0;
    const auto &Counters = It->second;
    uint32_t Result = 0;
    for (auto Cnt: Counters)
      if (Cnt)
        Result++;
    return Result;
  }

  Vector<double> FunctionWeights(size_t NumFunctions) const;
  void clear() { Functions.clear(); }

 private:

  typedef Vector<uint32_t> CoverageVector;

  uint32_t NumberOfCoveredBlocks(const CoverageVector &Counters) const {
    uint32_t Res = 0;
    for (auto Cnt : Counters)
      if (Cnt)
        Res++;
    return Res;
  }

  uint32_t NumberOfUncoveredBlocks(const CoverageVector &Counters) const {
    return Counters.size() - NumberOfCoveredBlocks(Counters);
  }

  uint32_t SmallestNonZeroCounter(const CoverageVector &Counters) const {
    assert(!Counters.empty());
    uint32_t Res = Counters[0];
    for (auto Cnt : Counters)
      if (Cnt)
        Res = Min(Res, Cnt);
    assert(Res);
    return Res;
  }

  // Function ID => vector of counters.
  // Each counter represents how many input files trigger the given basic block.
  std::unordered_map<size_t, CoverageVector> Functions;
  // Functions that have DFT entry.
  std::unordered_set<size_t> FunctionsWithDFT;
};

class DataFlowTrace {
 public:
  void ReadCoverage(const std::string &DirPath);
  bool Init(const std::string &DirPath, std::string *FocusFunction,
            Vector<SizedFile> &CorporaFiles, Random &Rand);
  void Clear() { Traces.clear(); }
  const Vector<uint8_t> *Get(const std::string &InputSha1) const {
    auto It = Traces.find(InputSha1);
    if (It != Traces.end())
      return &It->second;
    return nullptr;
  }

 private:
  // Input's sha1 => DFT for the FocusFunction.
  std::unordered_map<std::string, Vector<uint8_t> > Traces;
  BlockCoverage Coverage;
  std::unordered_set<std::string> CorporaHashes;
};
}  // namespace fuzzer

#endif // LLVM_FUZZER_DATA_FLOW_TRACE
