#ifndef COMPILER_INCLUDE
#define COMPILER_INCLUDE

#include "Options.h"

extern bool g_sEasyMode;

/// Struct to keep programs
struct CompilerPrograms
{
  std::string cc;      // C compiler
  std::string cxx;     // C++ compiler
  std::string ld;      // dynamic libs linker
  std::string lib;     // static libs linker
  std::string windres; // resource compiler
};

/// Struct to keep switches
struct CompilerSwitches
{
  std::string includeDirs;      // -I
  std::string resIncludeDirs;   // --include-dir
  std::string libDirs;          // -L
  std::string linkLibs;         // -l
  std::string libPrefix;        // lib
  std::string libExtension;     // a
  std::string defines;          // -D
  std::string resDefines;       // --define
  std::string genericSwitch;    // -
  std::string objectExtension;  // o
  bool forceLinkerUseQuotes;    // use quotes for filenames in linker command line (needed or not)?
  bool forceCompilerUseQuotes;  // use quotes for filenames in compiler command line (needed or not)?
  bool linkerNeedsLibPrefix;    // when adding a link library, linker needs prefix?
  bool linkerNeedsLibExtension; // when adding a link library, linker needs extension?
  bool supportsPCH;             // supports precompiled headers?
  std::string PCHExtension;     // precompiled headers extension
};

// -------------------------------------------------------------------------------------------------

/// Compiler abstract base class
class Compiler
{
  public:
  Compiler(const std::string& name)
      : m_name(name){};
  virtual ~Compiler(){};
  virtual void Process(Options& po, const CmdLineOptions& cl) = 0;

  std::string EasyMode(const std::string& str)
  {
    if(g_sEasyMode)
      return str;
    else
      return std::string();
  }

  std::string AddFlag(const std::string& flag)
  {
    if(flag.empty())
      return "";

    return flag + " ";
  }

  std::string AddLib(const std::string& lib)
  {
    if(lib.empty())
      return "";

    std::string result;
    result = m_switches.linkLibs;

    if(m_switches.linkerNeedsLibPrefix)
      result += m_switches.libPrefix;

    result += lib;

    if(m_switches.linkerNeedsLibExtension)
      result += "." + m_switches.libExtension;

    result += " ";

    return result;
  }

  std::string AddDefine(const std::string& define)
  {
    if(define.empty())
      return "";

    return m_switches.defines + define + " ";
  }

  std::string AddResDefine(const std::string& resDefine)
  {
    if(resDefine.empty())
      return "";

    return m_switches.resDefines + " " + resDefine + " ";
  }

  std::string AddIncludeDir(const std::string& includeDir)
  {
    if(includeDir.empty())
      return "";

    return m_switches.includeDirs + includeDir + " ";
  }

  std::string AddLinkerDir(const std::string& libDir)
  {
    if(libDir.empty())
      return "";

    return m_switches.libDirs + libDir + " ";
  }

  std::string AddResIncludeDir(const std::string& resIncludeDir)
  {
    if(resIncludeDir.empty())
      return "";

    return m_switches.resIncludeDirs + " " + resIncludeDir + " ";
  }
  /*
      std::string AddResLinkerDir(const std::string& resLibDir)
      {
          if (resLibDir.empty())
              return "";
          return m_switches.libDirs + libDir + " ";
      }*/

  std::string GetName() const
  {
    return m_name;
  }

  std::string GetAllLibs(Options& po)
  {
    std::string libs;
    libs += po["__WXLIB_ARGS_p"] + po["__WXLIB_OPENGL_p"] + po["__WXLIB_MEDIA_p"];
    libs += po["__WXLIB_STC_p"] + po["__WXLIB_WEBVIEW_p"];
    libs += po["__WXLIB_PROPGRID_p"] + po["__WXLIB_RIBBON_p"];
    libs += po["__WXLIB_RICH_p"];
    libs += po["__WXLIB_DBGRID_p"] + po["__WXLIB_ODBC_p"] + po["__WXLIB_XRC_p"];
    libs += po["__WXLIB_QA_p"] + po["__WXLIB_AUI_p"] + po["__WXLIB_HTML_p"] + po["__WXLIB_ADV_p"];
    libs += po["__WXLIB_CORE_p"] + po["__WXLIB_XML_p"] + po["__WXLIB_NET_p"];
    libs += po["__WXLIB_BASE_p"] + po["__WXLIB_MONO_p"];
    libs += po["__LIB_WXSCINTILLA_p"];
    libs += po["__LIB_TIFF_p"] + po["__LIB_JPEG_p"] + po["__LIB_PNG_p"];
    libs += po["__LIB_ZLIB_p"] + po["__LIB_REGEX_p"] + po["__LIB_EXPAT_p"];
    libs += po["EXTRALIBS_FOR_BASE"] + po["__UNICOWS_LIB_p"] + po["__GDIPLUS_LIB_p"];
    libs += po["__LIB_KERNEL32_p"] + po["__LIB_USER32_p"] + po["__LIB_GDI32_p"];
    libs += po["__LIB_COMDLG32_p"] + po["__LIB_REGEX_p"] + po["__LIB_WINSPOOL_p"];
    libs += po["__LIB_WINMM_p"] + po["__LIB_SHELL32_p"] + po["__LIB_COMCTL32_p"];
    libs += po["__LIB_OLE32_p"] + po["__LIB_OLEAUT32_p"] + po["__LIB_UUID_p"];
    libs += po["__LIB_RPCRT4_p"] + po["__LIB_ADVAPI32_p"] + po["__LIB_WSOCK32_p"];
    libs += po["__LIB_ODBC32_p"];

    return libs;
  }

  void GetVariablesValues(Options& po, const CmdLineOptions& cl, BuildFileOptions& cfg)
  {
    if(cl.KeyExists("--variable"))
    {
      std::string var = cl.KeyValue("--variable");

      if(po.KeyExists(var))
        po["variable"] += "PO: " + var + "=" + po[var] + "\n";
      else
        po["variable"] += "PO: " + var + " does not exist.\n";

      if(cfg.KeyExists(var))
        po["variable"] += "CFG: " + var + "=" + cfg[var] + "\n";
      else
        po["variable"] += "CFG: " + var + " does not exist.\n";
    }
  }

  protected:
  void ProcessStart(Options& po, const CmdLineOptions& cl, BuildFileOptions& cfg, SetupHOptions& sho);
  void ProcessMain(Options& po, const CmdLineOptions& cl, BuildFileOptions& cfg, SetupHOptions& sho);
  void ProcessEnd(Options& po, const CmdLineOptions& cl, BuildFileOptions& cfg);

  // set the following members in your class
  std::string m_name;
  CompilerPrograms m_programs;
  CompilerSwitches m_switches;

  private:
  Compiler(){};
};

// -------------------------------------------------------------------------------------------------

/// MinGW compiler
class CompilerMinGW : public Compiler
{
  public:
  CompilerMinGW()
      : Compiler("gcc")
  {
    m_programs.cc = "mingw32-gcc";
    m_programs.cxx = "mingw32-g++";
    m_programs.ld = "mingw32-g++ -shared -fPIC -o ";
    m_programs.lib = "ar.exe";
    m_programs.windres = "windres";

    m_switches.includeDirs = "-I";
    m_switches.resIncludeDirs = "--include-dir";
    m_switches.libDirs = "-L";
    m_switches.linkLibs = "-l";
    m_switches.libPrefix = "lib";
    m_switches.libExtension = "a";
    m_switches.defines = "-D";
    m_switches.resDefines = "--define";
    m_switches.genericSwitch = "-";
    m_switches.forceCompilerUseQuotes = false;
    m_switches.forceLinkerUseQuotes = false;
    m_switches.linkerNeedsLibPrefix = false;
    m_switches.linkerNeedsLibExtension = false;
    m_switches.supportsPCH = true;
    m_switches.PCHExtension = "h.gch";
  }

  void Process(Options& po, const CmdLineOptions& cl);
};

// -------------------------------------------------------------------------------------------------

/// Visual Studio compiler
class CompilerVC : public Compiler
{
  public:
  CompilerVC()
      : Compiler("vc")
  {
    m_programs.cc = "cl";
    m_programs.cxx = "cl";
    m_programs.ld = "link";
    m_programs.lib = "link";
    m_programs.windres = "rc";

    m_switches.includeDirs = "/I";
    m_switches.resIncludeDirs = "--include-dir"; ////////////
    m_switches.libDirs = "/LIBPATH:";
    m_switches.linkLibs = "";
    m_switches.libPrefix = "";
    m_switches.libExtension = "lib";
    m_switches.defines = "/D";
    m_switches.resDefines = "/d";
    m_switches.genericSwitch = "/";
    m_switches.forceCompilerUseQuotes = false;
    m_switches.forceLinkerUseQuotes = false;
    m_switches.linkerNeedsLibPrefix = false;
    m_switches.linkerNeedsLibExtension = true;
  }

  virtual void Process(Options& po, const CmdLineOptions& cl);
};

// -------------------------------------------------------------------------------------------------

/// DMars compiler
class CompilerDMC : public Compiler
{
  public:
  CompilerDMC()
      : Compiler("dmc")
  {
    m_programs.cc = "dmc";
    m_programs.cxx = "dmc";
    m_programs.ld = "link";
    m_programs.lib = "lib";
    m_programs.windres = "rcc";

    m_switches.includeDirs = "-I";
    m_switches.resIncludeDirs = "--include-dir"; ////////////
    m_switches.libDirs = "";
    m_switches.linkLibs = "";
    m_switches.libPrefix = "";
    m_switches.libExtension = "lib";
    m_switches.defines = "-D";
    m_switches.resDefines = "-D"; //////////////////////////////////(doesnt uses space)
    m_switches.genericSwitch = "-";
    m_switches.forceCompilerUseQuotes = false;
    m_switches.forceLinkerUseQuotes = true;
    m_switches.linkerNeedsLibPrefix = false;
    m_switches.linkerNeedsLibExtension = true;
  }

  virtual void Process(Options& po, const CmdLineOptions& cl);
};

// -------------------------------------------------------------------------------------------------

/// OpenWatcom compiler
class CompilerWAT : public Compiler
{
  public:
  CompilerWAT()
      : Compiler("wat")
  {
    m_programs.cc = "wcl386"; // TODO: wcc386
    m_programs.cxx = "wcl386"; // TODO: wpp386
    m_programs.ld = "wcl386";
    m_programs.lib = "wlib";
    m_programs.windres = "wrc";

    m_switches.includeDirs = "-i=";
    m_switches.resIncludeDirs = "-i=";
    m_switches.libDirs = "libp ";
    m_switches.linkLibs = "";
    m_switches.libPrefix = "libr ";
    m_switches.libExtension = "lib";
    m_switches.defines = "-d";
    m_switches.resDefines = "-d";
    m_switches.genericSwitch = "-";
    m_switches.forceCompilerUseQuotes = false;
    m_switches.forceLinkerUseQuotes = false;
    m_switches.linkerNeedsLibPrefix = true; // TODO:!
    m_switches.linkerNeedsLibExtension = true;
  }
  virtual void Process(Options& po, const CmdLineOptions& cl);
};

// -------------------------------------------------------------------------------------------------

/// Borland compiler
class CompilerBCC : public Compiler
{
  public:
  CompilerBCC()
      : Compiler("bcc")
  {
    m_programs.cc = "bcc32";
    m_programs.cxx = "bcc32";
    m_programs.ld = "ilink32";
    m_programs.lib = "tlib";
    m_programs.windres = "brcc32";

    m_switches.includeDirs = "-I";
    m_switches.resIncludeDirs = "--include-dir"; ////////////
    m_switches.libDirs = "-L";
    m_switches.linkLibs = "";
    m_switches.libPrefix = "";
    m_switches.libExtension = "lib";
    m_switches.defines = "-D";
    m_switches.resDefines = "/d"; ////////
    m_switches.genericSwitch = "-";
    m_switches.forceCompilerUseQuotes = false;
    m_switches.forceLinkerUseQuotes = true;
    m_switches.linkerNeedsLibPrefix = false;
    m_switches.linkerNeedsLibExtension = true;
  }

  void Process(Options& po, const CmdLineOptions& cl){};
};

#endif