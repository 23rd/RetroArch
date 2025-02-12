//
// Copyright (C) 2002-2005  3Dlabs Inc. Ltd.
// Copyright (C) 2013-2016 LunarG, Inc.
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above
//    copyright notice, this list of conditions and the following
//    disclaimer in the documentation and/or other materials provided
//    with the distribution.
//
//    Neither the name of 3Dlabs Inc. Ltd. nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
#ifndef _COMPILER_INTERFACE_INCLUDED_
#define _COMPILER_INTERFACE_INCLUDED_

#include "../Include/ResourceLimits.h"
#include "../MachineIndependent/Versions.h"

#include <cstring>
#include <vector>

#ifdef _WIN32
#define C_DECL __cdecl
#define SH_IMPORT_EXPORT
#else
#define SH_IMPORT_EXPORT
#ifndef __fastcall
#define __fastcall
#endif
#define C_DECL
#endif

//
// This is the platform independent interface between an OGL driver
// and the shading language compiler/linker.
//

#ifdef __cplusplus
    extern "C" {
#endif

// This should always increase, as some paths to do not consume
// a more major number.
// It should increment by one when new functionality is added.
#define GLSLANG_MINOR_VERSION 7

//
// Call before doing any other compiler/linker operations.
//
// (Call once per process, not once per thread.)
//
SH_IMPORT_EXPORT int ShInitialize();

//
// Call this at process shutdown to clean up memory.
//
SH_IMPORT_EXPORT int __fastcall ShFinalize();

//
// Types of languages the compiler can consume.
//
typedef enum {
    EShLangVertex,
    EShLangTessControl,
    EShLangTessEvaluation,
    EShLangGeometry,
    EShLangFragment,
    EShLangCompute,
    EShLangCount,
} EShLanguage;         // would be better as stage, but this is ancient now

typedef enum {
    EShLangVertexMask         = (1 << EShLangVertex),
    EShLangTessControlMask    = (1 << EShLangTessControl),
    EShLangTessEvaluationMask = (1 << EShLangTessEvaluation),
    EShLangGeometryMask       = (1 << EShLangGeometry),
    EShLangFragmentMask       = (1 << EShLangFragment),
    EShLangComputeMask        = (1 << EShLangCompute),
} EShLanguageMask;

namespace glslang {

class TType;

typedef enum {
    EShSourceNone,
    EShSourceGlsl,
    EShSourceHlsl,
} EShSource;                  // if EShLanguage were EShStage, this could be EShLanguage instead

typedef enum {
    EShClientNone,
    EShClientVulkan,
    EShClientOpenGL,
} EShClient;

typedef enum {
    EShTargetNone,
    EShTargetSpv,                 // preferred spelling
    EshTargetSpv = EShTargetSpv,  // legacy spelling
} EShTargetLanguage;

typedef enum {
    EShTargetVulkan_1_0 = (1 << 22),
    EShTargetVulkan_1_1 = (1 << 22) | (1 << 12),
    EShTargetOpenGL_450 = 450,
} EShTargetClientVersion;

typedef EShTargetClientVersion EshTargetClientVersion;

typedef enum {
    EShTargetSpv_1_0 = (1 << 16),
    EShTargetSpv_1_3 = (1 << 16) | (3 << 8),
} EShTargetLanguageVersion;

struct TInputLanguage {
    EShSource languageFamily; // redundant information with other input, this one overrides when not EShSourceNone
    EShLanguage stage;        // redundant information with other input, this one overrides when not EShSourceNone
    EShClient dialect;
    int dialectVersion;       // version of client's language definition, not the client (when not EShClientNone)
};

struct TClient {
    EShClient client;
    EShTargetClientVersion version;   // version of client itself (not the client's input dialect)
};

struct TTarget {
    EShTargetLanguage language;
    EShTargetLanguageVersion version; // version to target, if SPIR-V, defined by "word 1" of the SPIR-V header
    bool hlslFunctionality1;          // can target hlsl_functionality1 extension(s)
};

// All source/client/target versions and settings.
// Can override previous methods of setting, when items are set here.
// Expected to grow, as more are added, rather than growing parameter lists.
struct TEnvironment {
    TInputLanguage input;     // definition of the input language
    TClient client;           // what client is the overall compilation being done for?
    TTarget target;           // what to generate
};

const char* StageName(EShLanguage);

} // end namespace glslang

//
// Types of output the linker will create.
//
typedef enum {
    EShExVertexFragment,
    EShExFragment
} EShExecutable;

//
// Optimization level for the compiler.
//
typedef enum {
    EShOptNoGeneration,
    EShOptNone,
    EShOptSimple,       // Optimizations that can be done quickly
    EShOptFull,         // Optimizations that will take more time
} EShOptimizationLevel;

//
// Texture and Sampler transformation mode.
//
typedef enum {
    EShTexSampTransKeep,   // keep textures and samplers as is (default)
    EShTexSampTransUpgradeTextureRemoveSampler,  // change texture w/o embeded sampler into sampled texture and throw away all samplers
} EShTextureSamplerTransformMode;

//
// Message choices for what errors and warnings are given.
//
enum EShMessages {
    EShMsgDefault          = 0,         // default is to give all required errors and extra warnings
    EShMsgRelaxedErrors    = (1 << 0),  // be liberal in accepting input
    EShMsgSuppressWarnings = (1 << 1),  // suppress all warnings, except those required by the specification
    EShMsgAST              = (1 << 2),  // print the AST intermediate representation
    EShMsgSpvRules         = (1 << 3),  // issue messages for SPIR-V generation
    EShMsgVulkanRules      = (1 << 4),  // issue messages for Vulkan-requirements of GLSL for SPIR-V
    EShMsgOnlyPreprocessor = (1 << 5),  // only print out errors produced by the preprocessor
    EShMsgReadHlsl         = (1 << 6),  // use HLSL parsing rules and semantics
    EShMsgCascadingErrors  = (1 << 7),  // get cascading errors; risks error-recovery issues, instead of an early exit
    EShMsgKeepUncalled     = (1 << 8),  // for testing, don't eliminate uncalled functions
    EShMsgHlslOffsets      = (1 << 9),  // allow block offsets to follow HLSL rules instead of GLSL rules
    EShMsgDebugInfo        = (1 << 10), // save debug information
    EShMsgHlslEnable16BitTypes  = (1 << 11), // enable use of 16-bit types in SPIR-V for HLSL
    EShMsgHlslLegalization  = (1 << 12), // enable HLSL Legalization messages
};

//
// ShHandle held by but opaque to the driver.  It is allocated,
// managed, and de-allocated by the compiler/linker. It's contents
// are defined by and used by the compiler and linker.  For example,
// symbol table information and object code passed from the compiler
// to the linker can be stored where ShHandle points.
//
// If handle creation fails, 0 will be returned.
//
typedef void* ShHandle;

#ifdef __cplusplus
    }  // end extern "C"
#endif

////////////////////////////////////////////////////////////////////////////////////////////
//
// Deferred-Lowering C++ Interface
// -----------------------------------
//
// Below is a new alternate C++ interface, which deprecates the above
// opaque handle-based interface.
//
// The below is further designed to handle multiple compilation units per stage, where
// the intermediate results, including the parse tree, are preserved until link time,
// rather than the above interface which is designed to have each compilation unit
// lowered at compile time.  In the above model, linking occurs on the lowered results,
// whereas in this model intra-stage linking can occur at the parse tree
// (treeRoot in TIntermediate) level, and then a full stage can be lowered.
//

#include <list>
#include <string>

class TCompiler;
class TInfoSink;

namespace glslang {

class TIntermediate;
class TProgram;
class TPoolAllocator;

// Call this exactly once per process before using anything else
bool InitializeProcess();

// Call once per process to tear down everything
void FinalizeProcess();

// Resource type for IO resolver
enum TResourceType {
    EResSampler,
    EResTexture,
    EResImage,
    EResUbo,
    EResSsbo,
    EResUav,
    EResCount
};

// Make one TShader per shader that you will link into a program. Then
//  - provide the shader through setStrings()
//  - optionally call setEnv*(), see below for more detail
//  - call parse(): source language and target environment must be selected
//    either by correct setting of EShMessages sent to parse(), or by
//    explicitly calling setEnv*()
//  - query the info logs
//
// N.B.: Does not yet support having the same TShader instance being linked into
// multiple programs.
//
// N.B.: Destruct a linked program *before* destructing the shaders linked into it.
//
class TShader {
public:
    explicit TShader(EShLanguage);
    virtual ~TShader();
    void setStrings(const char* const* s, int n);

    // Interface to #include handlers.
    //
    // To support #include, a client of Glslang does the following:
    // 1. Call setStringsWithNames to set the source strings and associated
    //    names.  For example, the names could be the names of the files
    //    containing the shader sources.
    // 2. Call parse with an Includer.
    //
    // When the Glslang parser encounters an #include directive, it calls
    // the Includer's include method with the requested include name
    // together with the current string name.  The returned IncludeResult
    // contains the fully resolved name of the included source, together
    // with the source text that should replace the #include directive
    // in the source stream.  After parsing that source, Glslang will
    // release the IncludeResult object.
    class Includer {
    public:
        // An IncludeResult contains the resolved name and content of a source
        // inclusion.
        struct IncludeResult {
            IncludeResult(const std::string& headerName, const char* const headerData, const size_t headerLength, void* userData) :
                headerName(headerName), headerData(headerData), headerLength(headerLength), userData(userData) { }
            // For a successful inclusion, the fully resolved name of the requested
            // include.  For example, in a file system-based includer, full resolution
            // should convert a relative path name into an absolute path name.
            // For a failed inclusion, this is an empty string.
            const std::string headerName;
            // The content and byte length of the requested inclusion.  The
            // Includer producing this IncludeResult retains ownership of the
            // storage.
            // For a failed inclusion, the header
            // field points to a string containing error details.
            const char* const headerData;
            const size_t headerLength;
            // Include resolver's context.
            void* userData;
        protected:
            IncludeResult& operator=(const IncludeResult&);
            IncludeResult();
        };

        // For both include methods below:
        //
        // Resolves an inclusion request by name, current source name,
        // and include depth.
        // On success, returns an IncludeResult containing the resolved name
        // and content of the include.
        // On failure, returns a nullptr, or an IncludeResult
        // with an empty string for the headerName and error details in the
        // header field.
        // The Includer retains ownership of the contents
        // of the returned IncludeResult value, and those contents must
        // remain valid until the releaseInclude method is called on that
        // IncludeResult object.
        //
        // Note "local" vs. "system" is not an "either/or": "local" is an
        // extra thing to do over "system". Both might get called, as per
        // the C++ specification.

        // For the "system" or <>-style includes; search the "system" paths.
        virtual IncludeResult* includeSystem(const char* /*headerName*/,
                                             const char* /*includerName*/,
                                             size_t /*inclusionDepth*/) { return nullptr; }

        // For the "local"-only aspect of a "" include. Should not search in the
        // "system" paths, because on returning a failure, the parser will
        // call includeSystem() to look in the "system" locations.
        virtual IncludeResult* includeLocal(const char* /*headerName*/,
                                            const char* /*includerName*/,
                                            size_t /*inclusionDepth*/) { return nullptr; }

        // Signals that the parser will no longer use the contents of the
        // specified IncludeResult.
        virtual void releaseInclude(IncludeResult*) = 0;
        virtual ~Includer() {}
    };

    // Fail all Includer searches
    class ForbidIncluder : public Includer {
    public:
        virtual void releaseInclude(IncludeResult*) override { }
    };

    bool parse(const TBuiltInResource*, int defaultVersion, EProfile defaultProfile, bool forceDefaultVersionAndProfile,
               bool forwardCompatible, EShMessages, Includer&);

    bool parse(const TBuiltInResource* res, int defaultVersion, EProfile defaultProfile, bool forceDefaultVersionAndProfile,
               bool forwardCompatible, EShMessages messages)
    {
        TShader::ForbidIncluder includer;
        return parse(res, defaultVersion, defaultProfile, forceDefaultVersionAndProfile, forwardCompatible, messages, includer);
    }

    // Equivalent to parse() without a default profile and without forcing defaults.
    bool parse(const TBuiltInResource* builtInResources, int defaultVersion, bool forwardCompatible, EShMessages messages)
    {
        return parse(builtInResources, defaultVersion, ENoProfile, false, forwardCompatible, messages);
    }

    bool parse(const TBuiltInResource* builtInResources, int defaultVersion, bool forwardCompatible, EShMessages messages,
               Includer& includer)
    {
        return parse(builtInResources, defaultVersion, ENoProfile, false, forwardCompatible, messages, includer);
    }

    bool preprocess(const TBuiltInResource* builtInResources,
                    int defaultVersion, EProfile defaultProfile, bool forceDefaultVersionAndProfile,
                    bool forwardCompatible, EShMessages message, std::string* outputString,
                    Includer& includer);

    const char* getInfoLog();
    const char* getInfoDebugLog();

protected:
    TPoolAllocator* pool;
    EShLanguage stage;
    TCompiler* compiler;
    TIntermediate* intermediate;
    TInfoSink* infoSink;
    // strings and lengths follow the standard for glShaderSource:
    //     strings is an array of numStrings pointers to string data.
    //     lengths can be null, but if not it is an array of numStrings
    //         integers containing the length of the associated strings.
    //         if lengths is null or lengths[n] < 0  the associated strings[n] is
    //         assumed to be null-terminated.
    // stringNames is the optional names for all the strings. If stringNames
    // is null, then none of the strings has name. If a certain element in
    // stringNames is null, then the corresponding string does not have name.
    const char* const* strings;
    const int* lengths;
    const char* const* stringNames;
    int numStrings;

    TEnvironment environment;

    friend class TProgram;

private:
    TShader& operator=(TShader&);
};

// Make one TProgram per set of shaders that will get linked together.  Add all
// the shaders that are to be linked together.  After calling shader.parse()
// for all shaders, call link().
//
// N.B.: Destruct a linked program *before* destructing the shaders linked into it.
//
class TProgram {
public:
    TProgram();
    virtual ~TProgram();
    void addShader(TShader* shader) { stages[shader->stage].push_back(shader); }

    // Link Validation interface
    bool link(EShMessages);
    const char* getInfoLog();
    const char* getInfoDebugLog();

    TIntermediate* getIntermediate(EShLanguage stage) const { return intermediate[stage]; }
protected:
    bool linkStage(EShLanguage, EShMessages);

    TPoolAllocator* pool;
    std::list<TShader*> stages[EShLangCount];
    TIntermediate* intermediate[EShLangCount];
    bool newedIntermediate[EShLangCount];      // track which intermediate were "new" versus reusing a singleton unit in a stage
    TInfoSink* infoSink;
    bool linked;

private:
    TProgram(TProgram&);
    TProgram& operator=(TProgram&);
};

} // end namespace glslang

#endif // _COMPILER_INTERFACE_INCLUDED_
