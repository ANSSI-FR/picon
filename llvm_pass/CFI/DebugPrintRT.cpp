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

// DEBUG FUNCTION only
//
// retVal: Function to dump the return value (for ex, the call to @llvm.returnaddress)
// _M: The module
// Where: where to insert the printf call
//
//
void DEBUG_PRINT_ON_RUNTIME(llvm::Value *retVal, llvm::Module& _M, llvm::Instruction *Where)
{
  Function* func_printf = _M.getFunction("printf");
  ArrayType* arrType = ArrayType::get(IntegerType::get(_M.getContext(), 8), 12);
  GlobalVariable* glArrStrings = new GlobalVariable(_M, arrType, true,
      GlobalValue::PrivateLinkage, 0, ".str");

  Constant *constPrintfStr = ConstantDataArray::getString(_M.getContext(), "salut : %x\x0A", true);
  ConstantInt* magicGEP = ConstantInt::get(_M.getContext(), APInt(32, StringRef("0"), 10));

  std::vector<Constant*> ptrIdx;
  ptrIdx.push_back(magicGEP);
  ptrIdx.push_back(magicGEP);
  Constant* StrArgs = ConstantExpr::getGetElementPtr(glArrStrings, ptrIdx);

  glArrStrings->setInitializer(constPrintfStr);

  AllocaInst* ptr_i = new AllocaInst(IntegerType::get(_M.getContext(), 32), "i", Where);
  LoadInst* int32_12 = new LoadInst(ptr_i, "", false, Where);
  std::vector<Value*> Args;
  Args.push_back(StrArgs);
  Args.push_back(retVal);
  CallInst* int32_call = CallInst::Create(func_printf, Args, "call_printf_debug", Where);
}
