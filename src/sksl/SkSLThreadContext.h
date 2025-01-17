/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_THREADCONTEXT
#define SKSL_THREADCONTEXT

#include "include/core/SkTypes.h"
#include "include/private/SkSLDefines.h"
#include "include/private/SkSLProgramKind.h"
#include "include/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLMangler.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLProgram.h"

#include <list>
#include <memory>
#include <stack>
#include <string_view>
#include <vector>

#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#endif

namespace SkSL {

class Compiler;
class ModifiersPool;
class Pool;
class Position;
class ProgramElement;
class SymbolTable;
class Variable;
struct Modifiers;
struct ParsedModule;

namespace dsl {

class DSLCore;
class DSLWriter;

} // namespace dsl

/**
 * Thread-safe class that tracks per-thread state associated with SkSL output.
 */
class ThreadContext {
public:
    ThreadContext(SkSL::Compiler* compiler,  SkSL::ProgramKind kind,
              const SkSL::ProgramSettings& settings, SkSL::ParsedModule module, bool isModule);

    ~ThreadContext();

    /**
     * Returns true if the DSL has been started.
     */
    static bool IsActive();

    /**
     * Returns the Compiler used by DSL operations in the current thread.
     */
    static SkSL::Compiler& Compiler() { return *Instance().fCompiler; }

    /**
     * Returns the Context used by DSL operations in the current thread.
     */
    static SkSL::Context& Context();

    /**
     * Returns the Settings used by DSL operations in the current thread.
     */
    static const SkSL::ProgramSettings& Settings();

    /**
     * Returns the Program::Inputs used by the current thread.
     */
    static SkSL::Program::Inputs& Inputs() { return Instance().fInputs; }

    /**
     * Returns the collection to which DSL program elements in this thread should be appended.
     */
    static std::vector<std::unique_ptr<SkSL::ProgramElement>>& ProgramElements() {
        return Instance().fProgramElements;
    }

    static std::vector<const ProgramElement*>& SharedElements() {
        return Instance().fSharedElements;
    }

    /**
     * Returns the current SymbolTable.
     */
    static std::shared_ptr<SkSL::SymbolTable>& SymbolTable();

    /**
     * Returns the current memory pool.
     */
    static std::unique_ptr<Pool>& MemoryPool() { return Instance().fPool; }

    /**
     * Returns the current modifiers pool.
     */
    static std::unique_ptr<ModifiersPool>& GetModifiersPool() { return Instance().fModifiersPool; }

    /**
     * Returns the current ProgramConfig.
     */
    static const std::unique_ptr<ProgramConfig>& GetProgramConfig() { return Instance().fConfig; }

    static bool IsModule() { return GetProgramConfig()->fIsBuiltinCode; }

    /**
     * Returns the final pointer to a pooled Modifiers object that should be used to represent the
     * given modifiers.
     */
    static const SkSL::Modifiers* Modifiers(const SkSL::Modifiers& modifiers);

    struct RTAdjustData {
        // Points to a standalone sk_RTAdjust variable, if one exists.
        const Variable* fVar = nullptr;
        // Points to the interface block containing an sk_RTAdjust field, if one exists.
        const Variable* fInterfaceBlock = nullptr;
        // If fInterfaceBlock is non-null, contains the index of the sk_RTAdjust field within it.
        int fFieldIndex = -1;
    };

    /**
     * Returns a struct containing information about the RTAdjust variable.
     */
    static RTAdjustData& RTAdjustState();

    static const char* Filename() {
        return Instance().fFilename;
    }

    static void SetFilename(const char* filename) {
        Instance().fFilename = filename;
    }

    /**
     * Returns the ErrorReporter associated with the current thread. This object will be notified
     * when any DSL errors occur.
     */
    static ErrorReporter& GetErrorReporter() {
        return *Context().fErrors;
    }

    static void SetErrorReporter(ErrorReporter* errorReporter);

    /**
     * Notifies the current ErrorReporter that an error has occurred. The default error handler
     * prints the message to stderr and aborts.
     */
    static void ReportError(std::string_view msg, Position pos = {});

    /**
     * Forwards any pending errors to the DSL ErrorReporter.
     */
    static void ReportErrors(Position pos);

    static ThreadContext& Instance();

    static void SetInstance(std::unique_ptr<ThreadContext> instance);

private:
    class DefaultErrorReporter : public ErrorReporter {
        void handleError(std::string_view msg, Position pos) override;
    };

    void setupSymbolTable();

    std::unique_ptr<SkSL::ProgramConfig> fConfig;
    std::unique_ptr<SkSL::ModifiersPool> fModifiersPool;
    SkSL::Compiler* fCompiler;
    std::unique_ptr<Pool> fPool;
    SkSL::ProgramConfig* fOldConfig;
    SkSL::ModifiersPool* fOldModifiersPool;
    std::vector<std::unique_ptr<SkSL::ProgramElement>> fProgramElements;
    std::vector<const SkSL::ProgramElement*> fSharedElements;
    DefaultErrorReporter fDefaultErrorReporter;
    ErrorReporter& fOldErrorReporter;
    ProgramSettings fSettings;
    Mangler fMangler;
    RTAdjustData fRTAdjust;
    Program::Inputs fInputs;
    // for DSL error reporting purposes
    const char* fFilename = "";

#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
    struct StackFrame {
        GrFragmentProcessor::ProgramImpl* fProcessor;
        GrFragmentProcessor::ProgramImpl::EmitArgs* fEmitArgs;
        SkSL::StatementArray fSavedDeclarations;
    };
    std::stack<StackFrame, std::list<StackFrame>> fStack;
#endif // !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU

    friend class dsl::DSLCore;
    friend class dsl::DSLWriter;
};

} // namespace SkSL

#endif
