#include "clang/Basic/FileSystemOptions.h"
#include "clang/AST/ASTContext.h"
#include "clang/Driver/DriverDiagnostic.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/TextDiagnosticBuffer.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Basic/VirtualFileSystem.h"
#include "clang/Frontend/FrontendDiagnostic.h"
#include "clang/Lex/PreprocessorOptions.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/HeaderSearchOptions.h"
#include "clang/Driver/Options.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Lex/ModuleLoader.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/Support/Errc.h"
#include "llvm/Support/TargetSelect.h"
#include <iostream>
using namespace clang;
using namespace clang::vfs;
using namespace llvm;

class MyCompilerInstance : public ModuleLoader {
public:
	~MyCompilerInstance() override {};
	ModuleLoadResult loadModule(SourceLocation ImportLoc, ModuleIdPath Path,
                              Module::NameVisibilityKind Visibility,
                              bool IsInclusionDirective) override ;
	GlobalModuleIndex *loadGlobalModuleIndex(SourceLocation TriggerLoc) override ;

  	bool lookupMissingImports(StringRef Name, SourceLocation TriggerLoc) override;
  	void makeModuleVisible(Module *Mod, Module::NameVisibilityKind Visibility,
                         SourceLocation ImportLoc) override;
};

ModuleLoadResult
MyCompilerInstance::loadModule(SourceLocation ImportLoc,
                             ModuleIdPath Path,
                             Module::NameVisibilityKind Visibility,
                             bool IsInclusionDirective) {
	std::cout << "loadModule not implemented\n";
	exit(1);
}

GlobalModuleIndex *MyCompilerInstance::loadGlobalModuleIndex(SourceLocation TriggerLoc)
{
	std::cout << "loadGlobalModuleIndex not implemented\n";
	exit(1);
}

bool MyCompilerInstance::lookupMissingImports(StringRef Name, SourceLocation TriggerLoc)
{
	std::cout << "lookupMissingImports not implemented\n";
	exit(1);
}

void MyCompilerInstance::makeModuleVisible(Module *Mod, Module::NameVisibilityKind Visibility,
                         SourceLocation ImportLoc)
{
	std::cout << "makeModuleVisible not implemented\n";
	exit(1);
}

int main(int argc, char *argv[])
{
 	IntrusiveRefCntPtr<DiagnosticIDs> DiagID(new DiagnosticIDs());
 	IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts = new DiagnosticOptions();
  	TextDiagnosticPrinter *DiagsPrinter = new TextDiagnosticPrinter(llvm::errs(), &*DiagOpts);
  	DiagnosticsEngine Diags(DiagID, &*DiagOpts, DiagsPrinter);


  	IntrusiveRefCntPtr<vfs::FileSystem> FS = vfs::getRealFileSystem();
 	std::string File = "simple.c"; // No #include 's in c source file
  	StringRef InputFile = File;
  	FileSystemOptions *FileSystemOpts = new FileSystemOptions();
  	FileManager *FileMgr 		= new FileManager(*FileSystemOpts, FS);
  	SourceManager *SourceMgr 	= new SourceManager(Diags, *FileMgr);

  	const FileEntry *ModuleMap = FileMgr->getFile(InputFile, /*openFile*/true);
  	if (!ModuleMap)  {
    	Diags.Report(diag::err_module_map_not_found) << InputFile;
    	return 1;
  	}
  	SrcMgr::CharacteristicKind Kind = SrcMgr::C_User;
  	SourceMgr->setMainFileID(SourceMgr->createFileID(ModuleMap, SourceLocation(), Kind));

  	IntrusiveRefCntPtr<PreprocessorOptions> PPOpts(new PreprocessorOptions());
  	LangOptions			*LangOpts			= new LangOptions();
  	IntrusiveRefCntPtr<HeaderSearchOptions> HeaderSearchOpts(new HeaderSearchOptions());
  	std::shared_ptr<TargetOptions> 		TargetOpts(new TargetOptions());
  	TargetOpts->Triple 	= "x86_64-apple-macosx10.10.0";
  	TargetOpts->CPU		= "core2";
  	TargetInfo *Target = TargetInfo::CreateTargetInfo(Diags, TargetOpts);


  	PTHManager *PTHMgr = nullptr;

  	HeaderSearch *HeaderInfo = new HeaderSearch(HeaderSearchOpts,
                                              	*SourceMgr,
                                              	Diags,
                                              	*LangOpts,
                                              	Target);

  	MyCompilerInstance *CI 	= new MyCompilerInstance();
  	Preprocessor *PP 		= new Preprocessor(PPOpts, Diags, *LangOpts,
  												*SourceMgr, *HeaderInfo,
  												*CI, PTHMgr, true, TU_Complete);
  	PP->Initialize(*Target);

  	Token Tok;
  	PP->EnterMainSourceFile();
  	do {
    	PP->Lex(Tok);
    	PP->DumpToken(Tok, true);
    	llvm::errs() << "\n";
  	} while (Tok.isNot(tok::eof));
  	return 0;
}