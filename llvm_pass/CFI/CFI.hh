/* Copyright (C) 2015-2017 ANSSI

   This file is part of the Picon project.

   This file is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this file; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef CFI_H__
# define CFI_H__

#include <iostream>
#include <fstream>

#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Module.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringSet.h"

#include "llvm/Pass.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"

#include "DebugPrintRT.hh"
#include "CFIModulePass.hh"

using namespace llvm;

class CFIModulePass;
class ControlFlowIntegrity;

#define ANNOTATION_IGNORE_BLOCK "cfi_ignore_blocks\00"

struct CFIFct {
  llvm::Function* fun;
  uint32_t        id;
  std::map<llvm::BasicBlock*, uint32_t> bb_map;
};

class ControlFlowIntegrity {

  public:
    ControlFlowIntegrity(Module &M, const std::string& execName,
                                    const std::string& ipdom,
                                    const std::string& trans_table_file,
                                    const std::string& trans_tablebb_file,
                                    const std::string& ignoredFctF);

    void ParseAnnotations(void);

    bool isIgnoredFunction(const std::string &fName);
    bool isInitFunction(const std::string &fName);
    bool isFiniFunction(const std::string &fName);
    bool isIgnoredBlocksFunction(const std::string &fName);

    void writeFunctionIDs(void);

    void writeTransToFile(int granlvl);
    void writeCallgraph(std::fstream &fs);
    void writeAllowedBBTrans(std::fstream &fs);
    void writeIPDOMToFile(CFIModulePass* cfi);

    int IdentifyFunction(const Function& F);
    int AddIdentifiedFunction(const std::string name, uint32_t id);
    bool HasAlreadyBeenIdentified_id(const Function& F);
    int getIdentifier(Function *F);
    void IdentifyFunctions(void);
    void PatchCtorFunctions(void);
    void PatchDtorFunctions(void);

    void IdentifyBB(void);

  public:
    llvm::StringMap<CFIFct>::iterator fun_map_begin() { return _FunctionMap.begin(); }
    llvm::StringMap<CFIFct>::iterator fun_map_end() { return _FunctionMap.end(); }

  private:
    Module& _M;
    llvm::StringMap<CFIFct> _FunctionMap;
    //! _IdentifiedFunctions contains *all* known (local) functions and IDs, even from other files of the same binary
    llvm::StringMap<int> _IdentifiedFunctions;
    llvm::StringSet<> _IgnoredFunctions;
    llvm::StringSet<> _InitFunctions;
    llvm::StringSet<> _FiniFunctions;
    llvm::StringSet<> _IgnoredBlocksFunctions;
    std::string _fun_id_file;
    std::string _transTableFile;
    std::string _transTableBBFile;
    std::string _IgnoredFctFileName;
    std::string _ipdom_file;

    uint32_t _fun_id_ctr;
};

#endif // ! CFI_H__
