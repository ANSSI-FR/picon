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

#include <sstream>
#include <cstdio>

#include "CFI.hh"

class ValueOpRange {
  User &U;
  typedef User::value_op_iterator iter;
 public:
  ValueOpRange(User &U) : U(U) {}
  iter begin() { return U.value_op_begin(); }
  iter end() { return U.value_op_end(); }
};

ControlFlowIntegrity::ControlFlowIntegrity(Module &M,
                                           const std::string& fun_id_file,
                                           const std::string& ipdom_file,
                                           const std::string& trans_table_file,
                                           const std::string& trans_tablebb_file,
                                           const std::string& IgnoredFctFileName)
  : _M(M),
    _FunctionMap(),
    _IdentifiedFunctions(),
    _IgnoredFunctions(),
    _InitFunctions(),
    _FiniFunctions(),
    _IgnoredBlocksFunctions(),
    _fun_id_file(fun_id_file),
    _transTableFile(trans_table_file), _transTableBBFile(trans_tablebb_file),
    _IgnoredFctFileName(IgnoredFctFileName), _ipdom_file(ipdom_file),
    _fun_id_ctr(0) {
  if (!_IgnoredFctFileName.empty()) {
    std::fstream fs;
    fs.open(_IgnoredFctFileName, std::fstream::in);

    if (!fs.is_open()) {
      std::cerr << strerror(errno) << std::endl;
      assert(0 && "Can't open file to read the ignored functions\n");
    }

    std::string line;
    while (std::getline(fs, line)) {
      _IgnoredFunctions.insert(line);
    }
  }
}

bool ControlFlowIntegrity::isIgnoredFunction(const std::string &fName) {
  return (_IgnoredFunctions.find(fName) != _IgnoredFunctions.end());
}

bool ControlFlowIntegrity::isInitFunction(const std::string &fName) {
  return (_InitFunctions.find(fName) != _InitFunctions.end());
}

bool ControlFlowIntegrity::isFiniFunction(const std::string &fName) {
  return (_FiniFunctions.find(fName) != _FiniFunctions.end());
}

bool ControlFlowIntegrity::isIgnoredBlocksFunction(const std::string &fName) {
  return (_IgnoredBlocksFunctions.find(fName) != _IgnoredBlocksFunctions.end());
}

// writeTransToFile:
// This function must be called after IdentifyFunctions.
// This function is not reentrant, we need to lock the
// _transTableFile to handle multiple compilation jobs.
//
// It is responsible of writing functions and/or basic blocks
// transitions to the file specified by the option -trans-table
// of the `opt` command.
//
// In case of multiple source files (existing transition file), this function
// will append new discovered transition to the end of the file.
//
void ControlFlowIntegrity::writeTransToFile(int level) {
  std::fstream fs;

  fs.open(_transTableFile, std::fstream::out | std::fstream::app);

  if (!fs.is_open()) {
    std::cerr << strerror(errno) << std::endl;
    assert(0 && "Can't open file to write the transitions of functions.");
  }

  writeCallgraph(fs);
  fs.close();

  if (level == BASICBLOCK) {
    fs.open(_transTableBBFile, std::fstream::out | std::fstream::app);

    if (!fs.is_open()) {
      std::cerr << strerror(errno) << std::endl;
      assert(0 && "Can't open file to write the transitions of basic blocks.");
    }

    writeAllowedBBTrans(fs);

    fs.close();
  }
}

int ControlFlowIntegrity::IdentifyFunction(const Function& F) {
  uint32_t id;
  if (_IdentifiedFunctions.find(F.getName()) != _IdentifiedFunctions.end()) {
    id = _IdentifiedFunctions[F.getName()];
  } else {
    id = _fun_id_ctr++;
    _IdentifiedFunctions[F.getName()] = id;
  }
  return id;
}

int ControlFlowIntegrity::AddIdentifiedFunction(const std::string name, uint32_t id) {
  if (_IdentifiedFunctions.find(name) != _IdentifiedFunctions.end()) {
    std::cerr << "Function already present: " << name << std::endl;
    assert(0 && "Function already present");
  } else {
    _IdentifiedFunctions[name] = id;
  }
  if (_fun_id_ctr <= id)
    _fun_id_ctr = id + 1;
  return id;
}

bool ControlFlowIntegrity::HasAlreadyBeenIdentified_id(const Function&  F) {
  return (_IdentifiedFunctions.find(F.getName()) != _IdentifiedFunctions.end());
}

// writeFunctionIDs
// This function updates the file specified by the option -fun-id
// of the `opt` command with the list of function ID and known modules/flags.
//
// The format of the file is:
//   function_name function_identifier [Ignored|External|File]
//   where File is the -fun-id command line parameter.
//
void ControlFlowIntegrity::writeFunctionIDs(void) {
  uint32_t id;
  llvm::StringMap<std::pair<uint32_t,std::string>> oldMap;
  llvm::StringMap<bool> alreadyAdded;

  std::string fctName;
  std::string flags;
  std::ostringstream tmpbuf;
  std::ifstream fs(_fun_id_file);

  if (fs.good() == true) {
    // file was present, read previously known function IDs and flags
    if (!fs.is_open()) {
      report_fatal_error("Could not open the Function ID file", false);
      assert(0 && "Can't open the CFI function ID file.");
    }

    while (fs >> fctName >> id >> flags) {
      oldMap[fctName] = std::make_pair(id,flags);
    }
    fs.close();
  }

  for (auto &it : _FunctionMap) {
    std::string fct_name = it.getKey();
    CFIFct cfi_f = it.second;
    llvm::Function &F = *cfi_f.fun;
    std::string flags;

    id = cfi_f.id;

    // debug only check: new ID must be the same as the previous one
    if (oldMap.find(fct_name) != oldMap.end()) {
      std::pair<uint32_t,std::string> &E = oldMap[fct_name];
      assert(E.first == id && "ID of function in file is different from ID to be written");
    }

    if (isIgnoredFunction(fct_name)) {
      flags.append("Ignored");
      alreadyAdded[fct_name] = true;
    } else if (isInitFunction(fct_name)) {
      flags.append("Init");
      alreadyAdded[fct_name] = true;
    } else if (isFiniFunction(fct_name)) {
      flags.append("Fini");
      alreadyAdded[fct_name] = true;
    } else if (fct_name == "main") {
      flags.append("Main");
      alreadyAdded[fct_name] = true;
    } else if (F.isDeclaration()) {
      auto it = oldMap.find(fct_name);
      if (it != oldMap.end()) {
        if (it->getValue().second != "External") {
          // the old file knows more about the function - use it
          flags.append(it->getValue().second);
        } else {
          flags.append("External");
        }
      } else {
        flags.append("External");
      }
      alreadyAdded[fct_name] = true;
    } else if (HasAlreadyBeenIdentified_id(F)) {
      id = IdentifyFunction(F);
      alreadyAdded[fct_name] = true;
      flags.append(_fun_id_file);
    } else if (F.isDeclaration()) {
      assert(0 && "should not be here");
      flags.append(_fun_id_file);
    } else {
      assert(0 && "should not be here");
    }
    tmpbuf <<  fct_name.data() << " " << id  << " " <<  flags << "\n";
  }

  for (auto it=oldMap.begin(); it!=oldMap.end(); it++) {
    if (alreadyAdded.find(it->getKey()) == alreadyAdded.end()) {
      tmpbuf <<  it->getKey().data() << " " << it->getValue().first  << " " <<  it->getValue().second << "\n";
    }
  }

  std::ofstream ffs(_fun_id_file, std::fstream::out);
  // will overwrite previous _fun_id_file

  if (!ffs.is_open()) {
    std::cerr << strerror(errno) << std::endl;
    assert(0 && "Can't open file to write the CFI Function ID file.");
  }

  ffs << tmpbuf.str();
  ffs.close();
}

// getIdentifier:
// Returns the identifier for the given function.
// the function Map is constructed in IdentifyFunctions
// method.
//
int ControlFlowIntegrity::getIdentifier(Function *F) {
  if (_IdentifiedFunctions.find(F->getName()) != _IdentifiedFunctions.end()) {
    return _IdentifiedFunctions[F->getName()];
  }
  std::cerr << "getIdentifier out-of-range!\n";
  assert(0 && "Function Identifier out of range!");
  return -1;
}


// writeCallgraph:
// This function finds all call instructions of the module
// being processed. For each call instruction we retrieve
// the Function ID of the called function and save it to
// the file specified by @fs.
// @fs is a map with a function ID as a key.
//     The value of the map is a vector of
//     function ID that are called by the
//     function specified in the key.
// The format is:
//              (ID_FCT_0) ID_FCT_1 ID_FCT_3
//              (ID_FCT_1) ID_FCT_0 ID_FCT_2
// where ID_FCT_0 calls ID_FCT_1 and ID_FCT_3
//   and ID_FCT_1 calls ID_FCT_0 and ID_FCT_2.
//
void ControlFlowIntegrity::writeCallgraph(std::fstream &fs) {
  std::map<int, std::set<int>> _AllowedMapTrans;

  for (auto &it : _FunctionMap) {
    CFIFct cfi_f = it.second;
    llvm::Function &F = *cfi_f.fun;
    uint32_t id = cfi_f.id;

    if (F.isDeclaration())
      continue;

    for (auto &BB : F) {
      for (auto &I : BB) {
        if (isa<InvokeInst>(I)) {
          report_fatal_error("Invoke instructions not yet supported in Picon", false);
        }
        assert (!isa<InvokeInst>(I) && "Invoke instructions not supported");

        if (CallInst* CI = dyn_cast<CallInst>(&I)) {
          if (CI->isInlineAsm())
            continue;

          Function *CF = CI->getCalledFunction();

          // Check if dynamic call
          if (CF == NULL) {
            report_fatal_error("Indirect calls not yet supported in Picon", false);
          }
          assert (CF != NULL && "Indirect calls not supported");

          if (isIgnoredFunction(CF->getName()))
            continue;
          if (CF->getIntrinsicID() != Intrinsic::not_intrinsic)
            continue;


          // we have F -> CF
          // now find IDs of these functions
          if (_IdentifiedFunctions.find(CF->getName()) != _IdentifiedFunctions.end()) {
            uint32_t id_cf = _IdentifiedFunctions[CF->getName()];
            _AllowedMapTrans[id].insert(id_cf);
          }
          else {
            std::cerr << "Could not find ID for callee " << CF->getName().data() << ", called by " << F.getName().data() << std::endl;
          }
        }
      }
    }
  }

  // append map to file
  // note: previous content is not updated, the only functions added
  // are those from the current file
  // If this pass is called twice, information will be present twice
  for (auto &X : _AllowedMapTrans) {
    fs  << "(" << X.first << ") ";
    for (auto &XX : X.second)
      fs << XX << " ";
    fs << "\n";
  }
}

// WriteAllowedBBTrans:
// This functions looks for all basic blocks transition, builds a map,
// and writes it to the stream given as parameter.
//
void ControlFlowIntegrity::writeAllowedBBTrans(std::fstream &fs) {
  for (auto &it : _FunctionMap) {
    CFIFct cfi_f = it.second;
    llvm::Function &F = *cfi_f.fun;
    uint32_t fct_id = cfi_f.id;
    std::map<llvm::BasicBlock*, uint32_t> &bb_map = cfi_f.bb_map;

    if (isIgnoredFunction(F.getName()))
      continue;
    if (isIgnoredBlocksFunction(F.getName()))
      continue;
    if (F.isDeclaration())
      continue;

    unsigned int numBB  = F.size();

    fs << ";{" << fct_id << "} " << numBB << "\n";

    if (numBB == 1)
      continue;

    for (auto &BB : F) {
      auto it = succ_begin(&BB), et = succ_end(&BB);

      if (it == et)
        continue;

      fs << "(" << bb_map[&BB] << ") ";

      for (; it != et; ++it) {
        BasicBlock *successor = *it;

        fs << bb_map[successor] << " ";
      }
      fs << "\n";
    }

    fs << "\n";
  }
}

void ControlFlowIntegrity::IdentifyFunctions(void) {
  std::string is_defined;
  std::string fctName;
  uint32_t id, iid;

  ON_DEBUG(true, "IdentifyFunctions: " << _fun_id_file << "\n");
  std::ifstream fs(_fun_id_file);
  if (fs) {
    while (fs >> fctName >> iid >> is_defined) {
      (void)AddIdentifiedFunction(fctName,iid);
    }
    if (fs.peek()!=EOF) {
      report_fatal_error("IdentifyFunctions: bad input file", false);
      assert(0 && "IdentifyFunctions: bad input file");
    }
  }
  fs.close();

  for (auto &F : _M) {
    if (isIgnoredFunction(F.getName()))
      continue;

    if (F.getIntrinsicID() != Intrinsic::not_intrinsic)
      continue;

    id = IdentifyFunction(F);

    std::map<llvm::BasicBlock*, uint32_t> bb_map;
    uint32_t bb_id = 0;
    for (auto &BB : F) {
      bb_map[&BB] = bb_id++;
    }

    _FunctionMap[F.getName()] = {.fun=&F, .id=id, .bb_map=bb_map};
    ON_DEBUG(true, F.getName() << " with identifier " << id << "\n");
  }
}

// IdentifyBB:
// Since the identification for basic blocks is trivial
// and easily predictable, this function is only used
// for debugging purpose to dump the Basic Block Label
// and the attached Identifier.
void ControlFlowIntegrity::IdentifyBB(void) {
  for (auto &F : _M) {
    int Identifier = 0;
    if (!F.isDeclaration()) {
      errs() << "\n\nIdentifier BB of " << F.getName() << "\n\n";
      for (auto &BB : F) {
        errs() << BB.getName() << " with identifier " << Identifier <<
          " has " << BB.size() << " Instructions" << "\n";
        ++Identifier;
      }
    } else {
      errs() << F.getName() << " is a declaration\n";
    }
  }
  errs() << "\n";
}

// writeIPDOMToFile:
// This functions looks for the immediate post-dominator (IPD) of every basic
// block of every basic block of every function, and writes it to _ipdom_file
void ControlFlowIntegrity::writeIPDOMToFile(CFIModulePass* cfi) {
  std::fstream IPDOMFS;

  IPDOMFS.open(_ipdom_file, std::fstream::out | std::fstream::app);

  if (!IPDOMFS.is_open()) {
    std::cerr << strerror(errno) << std::endl;
    assert(0 && "Can't open file to write ipdom!");
  }

  for (auto &it : _FunctionMap) {
    CFIFct cfi_f = it.second;
    llvm::Function &F = *cfi_f.fun;
    std::map<llvm::BasicBlock*, uint32_t> &bb_map = cfi_f.bb_map;

    if (F.isDeclaration())
      continue ;
    if (isIgnoredFunction(F.getName()))
      continue;
    if (isIgnoredBlocksFunction(F.getName()))
      continue;

    PostDominatorTree &PDT = cfi->getAnalysis<PostDominatorTree>(F);
    for (BasicBlock &bb : F) {
      TerminatorInst* term = (bb).getTerminator();

      if (isa<InvokeInst>(term)) {
        report_fatal_error("Invoke Instruction detected!", false);
        continue;
      }

      BasicBlock* posdom = NULL;
      if (term->getNumSuccessors() >= 1) {
        succ_iterator si = succ_begin(&bb);
        posdom = *si;
        si++;
        for (succ_iterator se = succ_end(&bb); si != se; si++) {
          BasicBlock* su2 = *si;
          posdom = PDT.findNearestCommonDominator(posdom, su2);
          if (posdom == NULL)
            break;
        }
      }
      IPDOMFS << getIdentifier(&F) << " ";
      IPDOMFS  << bb_map[&(bb)] << " ";
      if (posdom != NULL) {
        IPDOMFS  << bb_map[posdom] << " ";
      } else {
        IPDOMFS  << "0 ";
      }
      IPDOMFS  << bb_map[&F.back()] << "\n";
    }
  }
  IPDOMFS.close();
}

void ControlFlowIntegrity::ParseAnnotations(void) {
  GlobalVariable *global_ctors = _M.getNamedGlobal("llvm.global.annotations");

  // check for ctor section
  if (!global_ctors || !global_ctors->getOperand(0)) {
    return;
  }

  Constant *c = global_ctors->getInitializer();
  if (!c)
    report_fatal_error("llvm.global.annotations without initializer!", false);

  ConstantArray *CA = dyn_cast<ConstantArray>(c);
  if (!CA)
    report_fatal_error("Cast to ConstantArray failed", true);

  for (Value *Op : ValueOpRange(*CA)) {
    ConstantStruct *CS = dyn_cast<ConstantStruct>(Op);
    if (!CS)
      report_fatal_error("Cast to ConstantStruct failed", true);

    Constant *FP = CS->getOperand(0);
    if (FP->isNullValue())
      break; // found a NULL termintator, stop here

    ConstantExpr *CE;
    Function *F = dyn_cast_or_null<Function>(FP);

    if (F == NULL) {
      // Strip off constant expression cast
      CE = dyn_cast<ConstantExpr>(FP);
      if (!CE)
        report_fatal_error("Cast to ConstantExpr failed", true);
      if (CE->isCast()) {
        FP = CE->getOperand(0);
        F = dyn_cast_or_null<Function>(FP);
      }
    }

    if (!F)
      report_fatal_error("Cast to Function failed", true);

    Constant *SP = CS->getOperand(1);
    if (SP->isNullValue())
      break; // found a NULL termintator, stop here

    // Strip off constant expression cast
    CE = dyn_cast<ConstantExpr>(SP);
    if (!CE)
      report_fatal_error("Cast to ConstantExpr failed", true);
    if (CE->isCast()) {
      SP = CE->getOperand(0);
    }

    Value *V = SP->getOperand(0);
    GlobalVariable *GV = dyn_cast_or_null<GlobalVariable>(V);
    if (!GV)
      report_fatal_error("Cast to GlobalVariable failed", false);
    assert(GV && "cast to GlobalVariable failed");

    Constant *cval = GV->getInitializer();
    const StringRef s = (cast<ConstantDataArray>(cval))->getAsCString();
    if (s == ANNOTATION_IGNORE_BLOCK) {
      _IgnoredBlocksFunctions.insert(F->getName());
    }
  }

  if (global_ctors->getNumUses() > 0)
    report_fatal_error("llvm.global.annotations uses count is > 0", false);
}

void ControlFlowIntegrity::PatchCtorFunctions(void) {
  GlobalVariable *global_ctors = _M.getNamedGlobal("llvm.global_ctors");

  // check for ctor section
  if (!global_ctors || !global_ctors->getOperand(0)) {
    return;
  }

  Constant *c = global_ctors->getInitializer();
  if (!c)
    report_fatal_error("llvm.global_ctors without initializer!", false);

  ConstantArray *CA = dyn_cast<ConstantArray>(c);
  if (!CA)
    report_fatal_error("Cast to ConstantArray failed", true);

  for (Value *Op : ValueOpRange(*CA)) {
    ConstantStruct *CS = dyn_cast<ConstantStruct>(Op);
    if (!CS)
      report_fatal_error("Cast to ConstantStruct failed", true);

    Constant *FP = CS->getOperand(1);
    if (FP->isNullValue())
      break; // found a NULL termintator, stop here

    Function *F = dyn_cast_or_null<Function>(FP);
    if (F == NULL) {
      // Strip off constant expression cast
      ConstantExpr *CE = dyn_cast<ConstantExpr>(FP);
      if (!CE)
        report_fatal_error("Cast to ConstantExpr failed", true);
      if (CE->isCast()) {
        FP = CE->getOperand(0);
      }
      F = dyn_cast_or_null<Function>(FP);
    }

    if (!F)
      report_fatal_error("Cast to Function failed", true);

    // set visibility to hidden (do not export), unless it is already
    // local (for ex. static), in which case we have to make it non-static
    if (F->hasLocalLinkage()) {
      F->setLinkage(llvm::GlobalValue::ExternalLinkage);
    }
    F->setVisibility(GlobalValue::HiddenVisibility);

    _InitFunctions.insert(F->getName());
  }

  if (global_ctors->getNumUses() > 0)
    report_fatal_error("llvm.global_ctors uses count is > 0!", false);

  global_ctors->removeFromParent();

}

void ControlFlowIntegrity::PatchDtorFunctions(void) {
  GlobalVariable *global_dtors = _M.getNamedGlobal("llvm.global_dtors");

  // check for dtor section
  if (!global_dtors || !global_dtors->getOperand(0)) {
    return;
  }

  Constant *c = global_dtors->getInitializer();
  if (!c)
    report_fatal_error("llvm.global_dtors without initializer!", false);

  ConstantArray *CA = dyn_cast<ConstantArray>(c);
  if (!CA)
    report_fatal_error("Cast to ConstantArray failed", true);

  for (Value *Op : ValueOpRange(*CA)) {
    ConstantStruct *CS = dyn_cast<ConstantStruct>(Op);
    if (!CS)
      report_fatal_error("Cast to ConstantStruct failed", true);

    Constant *FP = CS->getOperand(1);
    if (FP->isNullValue())
      break; // found a NULL termintator, stop here

    Function *F = dyn_cast_or_null<Function>(FP);
    if (F == NULL) {
      // Strip off constant expression cast
      ConstantExpr *CE = dyn_cast<ConstantExpr>(FP);
      if (!CE)
        report_fatal_error("Cast to ConstantExpr failed", true);
      if (CE->isCast()) {
        FP = CE->getOperand(0);
      }
      F = dyn_cast_or_null<Function>(FP);
    }

    if (!F)
      report_fatal_error("Cast to Function failed", true);

    // set visibility to hidden (do not export), unless it is already
    // local (for ex. static), in which case we have to make it non-static
    if (F->hasLocalLinkage()) {
      F->setLinkage(llvm::GlobalValue::ExternalLinkage);
    }
    F->setVisibility(GlobalValue::HiddenVisibility);

    _FiniFunctions.insert(F->getName());
  }

  if (global_dtors->getNumUses() > 0)
    report_fatal_error("llvm.global_dtors uses count is > 0!", false);

  global_dtors->removeFromParent();

}

