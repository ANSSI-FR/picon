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

#include "CFI.hh"
#include "CFIInjector.hh"

injectorCFI::injectorCFI(Module &M)
  : _M(M), _SavedRetAddr()  {
}

void injectorCFI::injectPrototype(void) {

  // extern pid_t __FORK_WRAPPER(void);

  std::vector<Type*>FuncTy_0_args;

  FuncTy_0_args.push_back(IntegerType::get(_M.getContext(), 32));
  FunctionType* FuncTy_0 = FunctionType::get(
      IntegerType::get(_M.getContext(), 32),
      IntegerType::get(_M.getContext(), 32),
      /*isVarArg=*/false);

  Function *func___FORK_WRAPPER = Function::Create(
      /*Type=*/FuncTy_0,
      /*Linkage=*/GlobalValue::ExternalLinkage,
      /*Name=*/"__FORK_WRAPPER", &_M); // (external, no body)


  // extern void __CFI_INTERNAL_BB_AFTER_BR(uint32_t f_id, uint32_t idBB);
  // extern void __CFI_INTERNAL_BB_BEFORE_BR(uint32_t f_id, uint32_t idBB);

  std::vector<Type*>FuncTy_1_args;

  FuncTy_1_args.push_back(IntegerType::get(_M.getContext(), 32));
  FuncTy_1_args.push_back(IntegerType::get(_M.getContext(), 32));
  FunctionType* FuncTy_1 = FunctionType::get(
      /*Result=*/Type::getVoidTy(_M.getContext()),
      /*Params=*/FuncTy_1_args,
      /*isVarArg=*/false);

  Function *func___CFI_INTERNAL_BB_AFTER_BR = Function::Create(
      /*Type=*/FuncTy_1,
      /*Linkage=*/GlobalValue::ExternalLinkage,
      /*Name=*/"__CFI_INTERNAL_BB_AFTER_BR", &_M); // (external, no body)

  Function *func___CFI_INTERNAL_BB_BEFORE_BR = Function::Create(
      /*Type=*/FuncTy_1,
      /*Linkage=*/GlobalValue::ExternalLinkage,
      /*Name=*/"__CFI_INTERNAL_BB_BEFORE_BR", &_M); // (external, no body)

  // extern void __CFI_INTERNAL_ENTER(uint32_t IdCurFct, void *retaddr);
  // extern void __CFI_INTERNAL_EXIT(uint32_t IdCurFct, void *retaddr);

  PointerType* PointerTy = PointerType::get(IntegerType::get(_M.getContext(), 8), 0);

  std::vector<Type*> FuncTy_2_args;
  FuncTy_2_args.push_back(IntegerType::get(_M.getContext(), 32));
  FuncTy_2_args.push_back(PointerTy);

  FunctionType* FuncTy_2 = FunctionType::get(
      /*Result=*/Type::getVoidTy(_M.getContext()),
      /*Params=*/FuncTy_2_args,
      /*isVarArg=*/false);

  Function* func___CFI_INTERNAL_ENTER = Function::Create(
      /*Type=*/FuncTy_2,
      /*Linkage=*/GlobalValue::ExternalLinkage,
      /*Name=*/"__CFI_INTERNAL_ENTER", &_M); // (external, no body)

  Function* func___CFI_INTERNAL_EXIT = Function::Create(
      /*Type=*/FuncTy_2,
      /*Linkage=*/GlobalValue::ExternalLinkage,
      /*Name=*/"__CFI_INTERNAL_EXIT", &_M); // (external, no body)

  // extern void __CFI_INTERNAL_CALL(uint32_t idFctCalled);
  // extern void __CFI_INTERNAL_RETURNED(uint32_t idFctCalled);
  // extern void __CFI_INTERNAL_FORK_SON(uint32_t idFctCalled);

  std::vector<Type*>FuncTy_3_args;

  FuncTy_3_args.push_back(IntegerType::get(_M.getContext(), 32));

  FunctionType* FuncTy_3 = FunctionType::get(
      /*Result=*/Type::getVoidTy(_M.getContext()),
      /*Params=*/FuncTy_3_args,
      /*isVarArg=*/false);

  Function *func___CFI_INTERNAL_CALL = Function::Create(
      /*Type=*/FuncTy_3,
      /*Linkage=*/GlobalValue::ExternalLinkage,
      /*Name=*/"__CFI_INTERNAL_CALL", &_M); // (external, no body)

  Function *func___CFI_INTERNAL_RETURNED = Function::Create(
      /*Type=*/FuncTy_3,
      /*Linkage=*/GlobalValue::ExternalLinkage,
      /*Name=*/"__CFI_INTERNAL_RETURNED", &_M); // (external, no body)

  Function *func___CFI_INTERNAL_FORK_SON= Function::Create(
      /*Type=*/FuncTy_3,
      /*Linkage=*/GlobalValue::ExternalLinkage,
      /*Name=*/"__CFI_INTERNAL_FORK_SON", &_M); // (external, no body)
}

void injectorCFI::injectMentry(Function& F, int FctID) {
  Function *retAddrFct = Intrinsic::getDeclaration(&_M, Intrinsic::returnaddress);
  Value *Zero = ConstantInt::get(Type::getInt32Ty(_M.getContext()), 0);
  Instruction *EntryPos = dyn_cast<Instruction>(F.getEntryBlock().getFirstInsertionPt());
  CallInst *retAddrCall = NULL;

  if (retAddrFct != NULL) {
    retAddrCall = CallInst::Create(retAddrFct, Zero, "saved_retaddr_prolog", EntryPos);
    _SavedRetAddr = retAddrCall;
  } else {
    assert(0 && "intrinsic returnaddress not found");
  }

  Function *funcEnter = _M.getFunction("__CFI_INTERNAL_ENTER");
  std::vector<Value*> Args;
  Value *ID = llvm::ConstantInt::get(_M.getContext(), llvm::APInt(32, FctID, true));
  Args.push_back(ID);
  Args.push_back(retAddrCall);

  if (funcEnter != NULL) {
    CallInst* callfctEnter = CallInst::Create(funcEnter, Args, "", EntryPos);
  } else {
    assert(0 && "__CFI_INTERNAL_ENTER must be defined");
  }
}

void injectorCFI::injectMexit(Function &F, int FctID) {
  Function *retAddrFct = Intrinsic::getDeclaration(&_M, Intrinsic::returnaddress);
  Value *Zero = ConstantInt::get(Type::getInt32Ty(_M.getContext()), 0);
  TerminatorInst *ExitPos = NULL;
  BasicBlock *Last;

  for (auto &BB : F) {
    Last = &BB;
  }

  ExitPos = Last->getTerminator();
  CallInst *retAddrCall = NULL;
  if (retAddrFct != NULL) {
    retAddrCall = CallInst::Create(retAddrFct, Zero, "saved_retaddr_epilog", ExitPos);
  } else {
    assert(0 && "intrinsic returnaddress not found");
  }

  Function *funcEnter = _M.getFunction("__CFI_INTERNAL_EXIT");
  std::vector<Value*> Args;
  Value *ID = llvm::ConstantInt::get(_M.getContext(), llvm::APInt(32, FctID, true));
  Args.push_back(ID);
  Args.push_back(retAddrCall);

  if (funcEnter != NULL) {
    CallInst* callfctEnter = CallInst::Create(funcEnter, Args, "", ExitPos);
  } else {
    assert(0 && "__CFI_INTERNAL_EXIT must be defined");
  }
}


void injectorCFI::injectForkSon(Function &F) {
  Function *funcforkWrpr = _M.getFunction("__FORK_WRAPPER");

  for (auto &BB : F) {
    for (auto &I : BB) {
      if (CallInst* CI = dyn_cast<CallInst>(&I)) {
        Function *NF = CI->getCalledFunction();
        if (NF->getName() == "fork") {
          std::vector<Value*> Args;
          Value *ID = llvm::ConstantInt::get(_M.getContext(), llvm::APInt(32, 42, true));
          Args.push_back(ID);
          CallInst* callfctEnter = CallInst::Create(funcforkWrpr, Args, "", CI);
          CI->replaceAllUsesWith(callfctEnter);
          CI->eraseFromParent();
          return ;
        }
      }
    }
  }
}

void injectorCFI::injectMcall(ControlFlowIntegrity *cfi, Function &F) {
  Instruction *BeforeCall;
  Instruction *AfterCall;;
  bool insertAfter = false;
  Function *NF = NULL;
  Function *funcCall = _M.getFunction("__CFI_INTERNAL_CALL");
  Function *funcReturned = _M.getFunction("__CFI_INTERNAL_RETURNED");

  if (funcCall == NULL)
    assert(0 && "__CFI_INTERNAL_CALL must be defined");

  if (funcReturned == NULL)
    assert(0 && "__CFI_INTERNAL_RETURNED must be defined");

  if (cfi->isIgnoredFunction(F.getName()) == true) {
    errs() << "don't inject INTERNAL_CALL & INTERNAL_RETURNED for " << F.getName() << "\n";
    return ;
  }

  for (auto &BB : F) {
    for (auto &I : BB) {
      if (insertAfter == true) {
        std::vector<Value*> Args;
        Value *ID = llvm::ConstantInt::get(_M.getContext(), llvm::APInt(32, cfi->getIdentifier(NF), true));
        Args.push_back(ID);
        CallInst* callfctEnter = CallInst::Create(funcReturned, Args, "", &I);
        insertAfter = false;
      }

      if (InvokeInst* _II = dyn_cast<InvokeInst>(&I)) {
        assert(0 && "Invoke not yet implemented");
      } else if (IntrinsicInst * _II = dyn_cast<IntrinsicInst>(&I)) {
        continue ;
      } else if (CallInst* CI = dyn_cast<CallInst>(&I)) {
        if (CI->isInlineAsm()) {
          errs() << "Picon warning: inline asm detected\n";
          continue;
        }
        NF = CI->getCalledFunction();
        if (NF == NULL) {
          CI->dump();
          report_fatal_error("Called function is NULL", true);
        }
        if (cfi->isIgnoredFunction(NF->getName()) == true) {
          continue ;
        }
        std::vector<Value*> Args;
        Value *ID = llvm::ConstantInt::get(_M.getContext(), llvm::APInt(32, cfi->getIdentifier(NF), true));
        Args.push_back(ID);
        CallInst* callfctEnter = CallInst::Create(funcCall, Args, "", CI);
        insertAfter = true;
      }
    }
  }
}

void injectorCFI::injectBBEnter(ControlFlowIntegrity *cfi, Function &F) {
  Function *funcBBEnter = _M.getFunction("__CFI_INTERNAL_BB_AFTER_BR");
  unsigned int idBB = 0;

  if (funcBBEnter == NULL)
    assert(0 && "__CFI_INTERNAL_BB_AFTER_BR must be defined");

  // n'injecte pas si le BB est le premier BB de la fonction
  for (auto &BB : F) {
    if (&BB == F.begin()) {
      ++idBB;
      continue ;
    }
    std::vector<Value*> Args;
    Value *ID = llvm::ConstantInt::get(_M.getContext(), llvm::APInt(32, cfi->getIdentifier(&F), true));
    Args.push_back(ID);
    ID = llvm::ConstantInt::get(_M.getContext(), llvm::APInt(32, idBB, true));
    Args.push_back(ID);
    CallInst* callfctBBEnter = CallInst::Create(funcBBEnter, Args, "", BB.getFirstNonPHIOrDbg());
    ++idBB;
  }
}

void injectorCFI::injectBBLeave(ControlFlowIntegrity *cfi, Function &F) {
  Function *funcBBLeave = _M.getFunction("__CFI_INTERNAL_BB_BEFORE_BR");
  unsigned int idBB = 0;

  if (funcBBLeave == NULL)
    assert(0 && "__CFI_INTERNAL_BB_BEFORE_BR must be defined");
  // injecte avant chaque instructions terminator
  // (ret, br, switch, indirectbr, invoke, resume, unreachable) la fonction
  // avec aucun parametre, afin de notifier le moniteur du nouvel état.
  // (a.k.a: attente d'un BBEnter)

  // n'injecte pas si le BB est le dernier BB de la fonction
  for (auto &BB : F) {
    if (&BB == &F.back()) {
      break ;
    }
    std::vector<Value*> Args;
    Value *ID = llvm::ConstantInt::get(_M.getContext(), llvm::APInt(32, cfi->getIdentifier(&F), true));
    Args.push_back(ID);
    ID = llvm::ConstantInt::get(_M.getContext(), llvm::APInt(32, idBB, true));
    Args.push_back(ID);
    CallInst *callfctBBLeave = CallInst::Create(funcBBLeave, Args, "", BB.getTerminator());
    ++idBB;
  }
}

void injectorCFI::InjectCode(ControlFlowIntegrity *cfi, int level) {

  injectPrototype();

  for (auto it = cfi->fun_map_begin(); it!=cfi->fun_map_end(); it++) {

    llvm::Function &F = *(it->getValue().fun);
    uint32_t id = it->getValue().id;

    injectMcall(cfi, F);
    if (F.isDeclaration())
      continue ;
    if (level == BASICBLOCK &&
        !cfi->isIgnoredBlocksFunction(F.getName()))
    {

      injectBBEnter(cfi, F);
      injectBBLeave(cfi, F);
    }
    injectMentry(F, id);
    injectMexit(F, id);
  }
}
