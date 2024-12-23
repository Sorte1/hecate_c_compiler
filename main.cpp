#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/GlobalVariable.h>

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

static std::vector<std::string> availableRegs = {
    "R5", "R4", "R3", "R2"
};
static std::unordered_map<const llvm::Value*, std::string> valueToReg;

static unsigned nextStringAddress = 2000;
static std::unordered_map<const llvm::GlobalVariable*, unsigned> globalStringBase;

static std::unordered_map<const llvm::AllocaInst*, std::string> allocaMap;
static int allocaCounter = 0;

static std::unordered_map<const llvm::BasicBlock*, std::string> blockNameMap;

static std::string getRegisterForValue(const llvm::Value* v) {
    if (valueToReg.find(v) != valueToReg.end()) {
        return valueToReg[v];
    }
    if (!availableRegs.empty()) {
        std::string r = availableRegs.back();
        availableRegs.pop_back();
        valueToReg[v] = r;
        return r;
    }
    valueToReg[v] = "R2";
    return "R2";
}

static std::string getOperandString(const llvm::Value *v) {
    using namespace llvm;
    if (auto *cint = dyn_cast<ConstantInt>(v)) {
        long long val = cint->getSExtValue();
        if (val == 0) return "R0";
        if (val == 1) return "R1";
        return std::to_string(val);
    }
    return getRegisterForValue(v);
}

static void assignAllocaLabels(const llvm::Function &func) {
    unsigned baseAddr = 1000;
    unsigned addrStep = 4;
    for (const llvm::BasicBlock &bb : func) {
        for (const llvm::Instruction &inst : bb) {
            if (auto *allocaInst = llvm::dyn_cast<llvm::AllocaInst>(&inst)) {
                unsigned numericAddress = baseAddr + allocaCounter * addrStep;
                std::string label = "@" + std::to_string(numericAddress);
                allocaMap[allocaInst] = label;
                allocaCounter++;
            }
        }
    }
}

static void emitGlobalStringsAsStoresWithRegister(const llvm::Module &M, std::ostream &out) {
    out << "globalinit:\n";
    const std::string TEMP_REG = "R5";

    for (auto &gvar : M.globals()) {
        if (gvar.hasInitializer()) {
            if (auto *cda = llvm::dyn_cast<llvm::ConstantDataArray>(gvar.getInitializer())) {
                if (cda->isString()) {
                    std::string strName = gvar.getName().str();
                    std::string strData = cda->getAsString().str();

                    unsigned baseAddr = nextStringAddress;
                    globalStringBase[&gvar] = baseAddr;

                    out << "  ; init string " << strName
                        << " \"" << strData.c_str() << "\" @"
                        << baseAddr << "\n";

                    for (size_t i = 0; i < strData.size(); i++) {
                        unsigned char c = (unsigned char)strData[i];
                        out << "  load " << TEMP_REG << ", " << (unsigned)c << "\n";
                        out << "  storeByte @" << (baseAddr + i) << ", " << TEMP_REG << "\n";
                    }
                    nextStringAddress += (unsigned)strData.size();
                }
            }
        }
    }
    out << "  jmp @main\n\n";
}

static std::string getMemoryAddress(const llvm::Value *ptrVal) {
    ptrVal = ptrVal->stripPointerCasts();

    if (auto *ai = llvm::dyn_cast<llvm::AllocaInst>(ptrVal)) {
        if (allocaMap.find(ai) != allocaMap.end()) {
            return allocaMap[ai];
        }
    }
    if (auto *gv = llvm::dyn_cast<llvm::GlobalVariable>(ptrVal)) {
        auto it = globalStringBase.find(gv);
        if (it != globalStringBase.end()) {
            return "@" + std::to_string(it->second);
        }
        return "@" + gv->getName().str();
    }
    if (ptrVal->hasName()) {
        return "@" + ptrVal->getName().str();
    }
    return "@unknown_mem";
}

static void assignBlockNames(const llvm::Function &func) {
    blockNameMap.clear();
    int blockCounter = 0;
    for (const llvm::BasicBlock &bb : func) {
        if (bb.hasName()) {
            blockNameMap[&bb] = bb.getName().str();
        } else {
            blockNameMap[&bb] = "Block" + std::to_string(blockCounter++);
        }
    }
}

static void emitGlobalStrings(const llvm::Module &module, std::ostream &out) {
    out << "; --- Global Data Section ---\n";
    for (const llvm::GlobalVariable &gvar : module.globals()) {
        if (gvar.hasInitializer()) {
            if (auto *cda = llvm::dyn_cast<llvm::ConstantDataArray>(gvar.getInitializer())) {
                if (cda->isString()) {
                    std::string name = gvar.getName().str();
                    std::string strVal = cda->getAsString().str();
                    out << ";" << name << ":\n";
                    out << ";" << "  db  ";
                    for (size_t i = 0; i < strVal.size(); i++) {
                        if (i > 0) out << ",";
                        out  << (int)(unsigned char)strVal[i];
                    }
                    out << " ; \"" <<  strVal.c_str() << "\"" << "\n";
                }
            }
        }
    }
    out << "; --- End of Global Data ---\n\n";
}

static void convertInstruction(const llvm::Instruction &inst, std::ostream &output) {
    using namespace llvm;
    std::string opcodeName = inst.getOpcodeName();

    if (opcodeName == "alloca") {
        if (auto *A = dyn_cast<AllocaInst>(&inst)) {
            output << "  ; local space: " << allocaMap[A] << "\n";
        } else {
            output << "  ; unknown alloca\n";
        }
        return;
    }

    if (opcodeName == "store") {
        const Value *val = inst.getOperand(0);
        const Value *ptr = inst.getOperand(1);
        val = val->stripPointerCasts();
        ptr = ptr->stripPointerCasts();
        std::string memAddr = getMemoryAddress(ptr);
        std::string src = getOperandString(val);
        bool isNum = !src.empty() && std::all_of(src.begin(), src.end(), ::isdigit);

        if (isNum) {
            output << "  load R5, " << src << "\n";
            output << "  store " << memAddr << ", R5\n";
        } else {
            output << "  store " << memAddr << ", " << src << "\n";
        }
        return;
    }

    if (opcodeName == "load") {
        const Value *ptr = inst.getOperand(0);
        std::string addr    = getMemoryAddress(ptr);
        std::string destReg = getRegisterForValue(&inst);
        output << "  load " << destReg << ", " << addr << "\n";
        return;
    }

    if (auto *callInst = dyn_cast<CallInst>(&inst)) {
        if (auto *func = callInst->getCalledFunction()) {
            if (func->getName() == "inspect") {
                if (callInst->arg_size() > 0) {
                    const Value *arg0 = callInst->getArgOperand(0);
                    std::string memAddr = getMemoryAddress(arg0);
                    output << "  inspect " << memAddr << " ; debug\n";
                } else {
                    output << "  inspect @unknown_mem ; debug\n";
                }
            } else {
                output << "  call @" << func->getName().str() << "\n";
            }
        } else {
            output << "  ; indirect call?\n";
        }
        return;
    }

    if (opcodeName == "ret") {
        if (inst.getNumOperands() > 0) {
            std::string retStr = getOperandString(inst.getOperand(0));
            output << "  ; returning " << retStr << "\n";
        }
        output << "  ret\n";
        return;
    }

    if (opcodeName == "br") {
        const BranchInst *br = dyn_cast<BranchInst>(&inst);
        if (!br) {
            output << "  ; unknown branch\n";
            return;
        }
        if (br->isUnconditional()) {
            const BasicBlock *dest = br->getSuccessor(0);
            output << "  jmp @" << blockNameMap[dest] << "\n";
        } else {
            std::string condReg = getOperandString(br->getCondition());
            const BasicBlock *trueBB  = br->getSuccessor(0);
            const BasicBlock *falseBB = br->getSuccessor(1);
            output << "  cmp " << condReg << ", R0\n";
            output << "  je @" << blockNameMap[falseBB] << "\n";
            output << "  jmp @" << blockNameMap[trueBB]  << "\n";
        }
        return;
    }

    if (opcodeName == "icmp") {
        const ICmpInst *icmp = dyn_cast<ICmpInst>(&inst);
        std::string lhs = getOperandString(icmp->getOperand(0));
        std::string rhs = getOperandString(icmp->getOperand(1));
        std::string predName = ICmpInst::getPredicateName(icmp->getPredicate()).str();
        output << "  ; icmp " << predName << " " << lhs << ", " << rhs << "\n";
        output << "  cmp " << lhs << ", " << rhs << "\n";
        return;
    }

    if (opcodeName == "add" || opcodeName == "sub" ||
        opcodeName == "mul" || opcodeName == "udiv")
    {
        std::string lhs = getOperandString(inst.getOperand(0));
        std::string rhs = getOperandString(inst.getOperand(1));
        std::string destReg = getRegisterForValue(&inst);

        if (opcodeName == "add") {
            output << "  ; plus " << destReg << " = " << lhs << " + " << rhs << "\n";
            output << "  load " << destReg << ", " << lhs << "\n";
            output << "  add " << destReg << ", " << rhs << "\n";
        } else if (opcodeName == "sub") {
            output << "  ; minus " << destReg << " = " << lhs << " - " << rhs << "\n";
            output << "  load " << destReg << ", " << lhs << "\n";
            output << "  sub " << destReg << ", " << rhs << "\n";
        } else if (opcodeName == "mul") {
            output << "  ; multiply " << destReg << " = " << lhs << " * " << rhs << "\n";
            output << "  load " << destReg << ", " << lhs << "\n";
            output << "  mul " << destReg << ", " << rhs << "\n";
        } else if (opcodeName == "udiv") {
            output << "  ; divide " << destReg << " = " << lhs << " / " << rhs << "\n";
            output << "  load " << destReg << ", " << lhs << "\n";
            output << "  div " << destReg << ", " << rhs << "\n";
        }
        return;
    }

    output << "  ; not handled: " << opcodeName << "\n";
}

static void convertBasicBlock(const llvm::BasicBlock &bb, std::ostream &output) {
    std::string label = blockNameMap[&bb];
    output << label << ":\n";
    for (const llvm::Instruction &inst : bb) {
        convertInstruction(inst, output);
    }
}

static void convertFunction(const llvm::Function &func, std::ostream &output) {
    assignBlockNames(func);
    assignAllocaLabels(func);
    output << func.getName().str() << ":\n";
    for (const llvm::BasicBlock &bb : func) {
        convertBasicBlock(bb, output);
    }
    if (func.getName() == "main") {
        output << "  halt ; main ended\n";
    }
}

static void convertLLVMToCustomAssembly(const std::string &filename, std::ostream &output) {
    llvm::LLVMContext context;
    llvm::SMDiagnostic err;
    auto module = llvm::parseIRFile(filename, err, context);
    if (!module) {
        llvm::errs() << "Parsing IR failed: " << filename << "\n";
        err.print("convertLLVMToCustomAssembly", llvm::errs());
        return;
    }

    availableRegs = {"R5","R4","R3","R2"};
    valueToReg.clear();
    blockNameMap.clear();
    allocaMap.clear();
    allocaCounter = 0;
    nextStringAddress = 2000;
    globalStringBase.clear();

    emitGlobalStrings(*module, output);

    // emitGlobalStringsAsStoresWithRegister(*module, output);

    // convert each function
    for (const llvm::Function &func : *module) {
        if (!func.isDeclaration()) {
            convertFunction(func, output);
        }
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        llvm::errs() << "Usage: ./hecate_c_compiler_c <file.ll> [-o <output_file>]\n";
        return 1;
    }

    std::string inputFile;
    std::string outputFile;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "-o" && i + 1 < argc) {
            outputFile = argv[++i];
        } else {
            inputFile = argv[i];
        }
    }

    if (inputFile.empty()) {
        llvm::errs() << "No input file provided\n";
        return 1;
    }

    std::ofstream fileStream;
    std::ostream *outputStream = &std::cout;
    if (!outputFile.empty()) {
        fileStream.open(outputFile);
        if (!fileStream.is_open()) {
            llvm::errs() << "Cannot open output file: " << outputFile << "\n";
            return 1;
        }
        outputStream = &fileStream;
    }

    convertLLVMToCustomAssembly(inputFile, *outputStream);

    if (fileStream.is_open()) {
        fileStream.close();
    }
    return 0;
}
