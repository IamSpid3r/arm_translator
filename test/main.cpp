#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <string>
#include <sys/mman.h>
#include <memory>
#include "IRContextInternal.h"
#include "RegisterInit.h"
#include "RegisterAssign.h"
#include "Check.h"
#include "log.h"

extern "C" {
void yyparse(IRContext*);
typedef void* yyscan_t;
int yylex_init(yyscan_t* scanner);

int yylex_init_extra(struct IRContext* user_defined, yyscan_t* scanner);

int yylex_destroy(yyscan_t yyscanner);
void vex_disp_run_translations(uintptr_t* two_words,
    void* guest_state,
    void* host_addr);
void yyset_in  (FILE * in_str ,yyscan_t yyscanner );
void vex_disp_cp_chain_me_to_slowEP(void);
void vex_disp_cp_chain_me_to_fastEP(void);
void vex_disp_cp_xindir(void);
void vex_disp_cp_xassisted(void);
void vex_disp_cp_evcheck_fail(void);
}

static void initGuestState(CPUARMState& state, const IRContextInternal& context)
{
    memset(&state, 0, sizeof(state));
    RegisterAssign assign;
    for (auto&& ri : context.m_registerInit) {
        ri.m_control->reset();
        uintptr_t val = ri.m_control->init();
        assign.assign(&state, ri.m_name, val);
    }
}

static void checkRun(const char* who, const IRContextInternal& context, const uintptr_t* twoWords, const CPUARMState& guestState)
{
    unsigned checkPassed = 0, checkFailed = 0, count = 0;
    LOGE("checking %s...\n", who);
    for (auto& c : context.m_checks) {
        std::string info;
        if (c->check(&guestState, twoWords, info)) {
            checkPassed++;
        }
        else {
            checkFailed++;
        }
        LOGE("[%u]:%s.\n", ++count, info.c_str());
    }
    LOGE("passed %u, failed %u.\n", checkPassed, checkFailed);
}

int main(int argc, char** argv)
{
    if (argc <= 1) {
        LOGE("need one arg.");
        exit(1);
    }
    FILE* inputFile = fopen(argv[1], "r");
    if (!inputFile) {
        LOGE("fails to open input file.");
        exit(1);
    }
    IRContextInternal context;
    yylex_init_extra(&context, &context.m_scanner);
    yyset_in(inputFile, &context.m_scanner);
    yyparse(&context);
    yylex_destroy(context.m_scanner);

    // allocate executable memory
    static const size_t execMemSize = 4096;
    void* execMem = mmap(nullptr, execMemSize, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    EMASSERT(execMem != MAP_FAILED);

    // run with llvm
    ARMCPU cpu;
    // FIXME: translate here and copy to code to execMem
    initGuestState(cpu.env, context);
    uintptr_t twoWords[2];
    vex_disp_run_translations(twoWords, &cpu.env, execMem);
    checkRun("llvm", context, twoWords, cpu.env);
    return 0;
}
