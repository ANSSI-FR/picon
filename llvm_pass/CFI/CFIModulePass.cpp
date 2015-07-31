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

#include "CFI.hh"
#include "CFIInjector.hh"
#include "CFIModulePass.hh"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/file.h>

static cl::opt<int> level("cfi-level", cl::desc("Specify the CFI instrumentation level: [0/1] 0: functions, 1: functions and basic blocks"), cl::value_desc("cfi-level"));
static cl::opt<std::string> prefix("cfi-prefix", cl::desc("Specify the prefix used to load/store CFI temporary files"), cl::value_desc("[dirname/]basename"));
static cl::opt<std::string> IgnoredFctFileName("cfi-ignore", cl::desc("Specify file containing functions ignored by CFI"), cl::value_desc("filename"));

CFIModulePass::CFIModulePass() : ModulePass(ID) { }

bool CFIModulePass::runOnModule(Module &M) {

  std::string s_fctid, s_trans, s_bbtrans, s_ipdom;
  std::string s_lock;
  int fd, rc;

  s_fctid = prefix + ".fctid";
  s_trans = prefix + ".trans";
  s_bbtrans = prefix + ".bbtrans";
  s_ipdom = prefix + ".ipdom";
  s_lock = prefix + ".lock";

  fd = open(s_lock.c_str(), O_RDWR | O_CREAT, 0666);
  if (fd < 0) {
    errs() << "Could not create lock file\n";
    assert( fd < 0 && "Could not create lock file\n");
  }
  rc = flock(fd, LOCK_EX);
  if (rc < 0) {
    errs() << "flock() failed\n";
    assert( rc < 0 && "flock() failed\n");
  }

  ControlFlowIntegrity RCFI(M, s_fctid, s_ipdom, s_trans, s_bbtrans, IgnoredFctFileName);

  RCFI.ParseAnnotations();

  RCFI.IdentifyFunctions();

#ifndef NDEBUG
  RCFI.IdentifyBB();
#endif // !NDEBUG

  RCFI.PatchCtorFunctions();
  RCFI.PatchDtorFunctions();

  RCFI.writeTransToFile(level);
  RCFI.writeFunctionIDs();
  RCFI.writeIPDOMToFile(this);


  (void)flock(fd, LOCK_UN);

   injectorCFI ICFI(M);
   ICFI.InjectCode(&RCFI, level);

  return true; // module was modified
}

void CFIModulePass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<PostDominatorTree>();
}

char CFIModulePass::ID = 0;
static RegisterPass<CFIModulePass>    U("cfi", "control flow integrity");
