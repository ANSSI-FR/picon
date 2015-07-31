/* Copyright (C) 2015 ANSSI

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

#ifndef INJECTOR_H__
# define INJECTOR_H__

#include <iostream>

#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"

class injectorCFI {
  public:
    injectorCFI(Module &M);

  public:
    void injectPrototype(void);
    void injectMentry(Function& F, int FctID);
    void injectMexit(Function &F, int FctID);
    void injectMcall(ControlFlowIntegrity *, Function &F);
    void injectBBEnter(ControlFlowIntegrity *cfi, Function &F);
    void injectBBLeave(ControlFlowIntegrity *cfi, Function &F);
    void injectForkSon(Function &F);

  public:
    void InjectCode(ControlFlowIntegrity *, int granlvl);

  private:
    Module& _M;
    Value* _SavedRetAddr;
};

#endif // !INJECTOR_H__
