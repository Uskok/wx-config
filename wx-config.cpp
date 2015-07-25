////////////////////////////////////////////////////////////////////////////////////////////////////
// Name:        wx-config.cpp
// Purpose:     A wx-config implementation for Windows
// Author:      Takeshi Miya
// Created:     2006-03-23
// Copyright:   (c) Takeshi Miya
// Licence:     wxWidgets licence
// $Rev: 26 $
// $URL: http://wx-config-win.googlecode.com/svn/trunk/wx-config-win.cpp $
// $Date: 2006-12-21 04:08:16 +0200 (Thu, 21 Dec 2006) $
// $Id: wx-config-win.cpp 26 2006-12-21 02:08:16Z takeshimiya $
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Compiler.h"

const std::string g_tokError = "wx-config Error: ";
const std::string g_tokWarning = "wx-config Warning: ";
bool g_sEasyMode = false;

// -------------------------------------------------------------------------------------------------

std::string GetSvnRevision()
{
  std::string str = "$Rev: 26 $";
  if(str.length() > 8)
    return str.substr(6, str.length() - 8);
  else
    return "X";
}

std::string GetSvnDate()
{
  std::string str = "$Date: 2006-12-21 04:08:16 +0200 (Thu, 21 Dec 2006) $";
  if(str.length() > 16)
    return str.substr(7, 10);
  else
    return "2006-XX-XX";
}

// -------------------------------------------------------------------------------------------------

void CheckEasyMode(CmdLineOptions& cl)
{
  if(cl.KeyExists("--easymode"))
  {
    if(cl["--easymode"] == "no")
      g_sEasyMode = false;
    else if(cl["--easymode"] == "yes" || cl["--easymode"].empty())
      g_sEasyMode = true;
  }
}

// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------

void NormalizePath(std::string& path)
{
  // converts all slashes to backslashes
  std::replace(path.begin(), path.end(), '/', '/');

  // removes the first slash (if any) from the given path
  std::string::iterator firstChar = path.begin();
  if(*firstChar == '/')
    path.erase(firstChar);

  // removes the last slash (if any) from the given path
  std::string::iterator lastChar = path.end() - 1;
  if(*lastChar == '/')
    path.erase(lastChar);
}

// -------------------------------------------------------------------------------------------------

/// Return true if string changed
bool ReplaceCompilerIfFound(std::string& wxcfg, const std::string& compiler)
{
  size_t loc;
  loc = wxcfg.find(compiler);
  if(loc != std::string::npos)
    return false;

  std::vector<std::string> compilers;
  compilers.push_back("gcc_");
  compilers.push_back("dmc_");
  compilers.push_back("vc_");
  compilers.push_back("wat_");
  compilers.push_back("bcc_");

  for(std::vector<std::string>::const_iterator it = compilers.begin(); it != compilers.end(); ++it)
  {
    loc = wxcfg.find(*it);
    if(loc != std::string::npos)
    {
      wxcfg.replace(loc, (*it).length(), compiler + "_");
      return true;
    }
  }
  return false;
}

// -------------------------------------------------------------------------------------------------

/// Return true if string changed
bool ReplaceUniv(std::string& wxcfg, bool enable)
{
  const std::string univStr("univ");
  size_t univPos = wxcfg.rfind(univStr);

  if(enable)
  {
    /// Pattern: Replace /(msw|base)/ to /(msw|base)univ/
    if(univPos == std::string::npos)
    {
      const std::string mswStr("msw");
      size_t mswPos = wxcfg.rfind(mswStr);
      if(mswPos != std::string::npos)
        wxcfg.insert(mswPos + mswStr.length(), univStr);
      else
      {
        const std::string baseStr("base");
        size_t basePos = wxcfg.rfind(baseStr);
        if(basePos != std::string::npos)
          wxcfg.insert(basePos + baseStr.length(), univStr);
      }
      return true;
    }
  }
  else
  {
    /// Pattern: Remove /univ/ in /(msw|base)univ/
    if(univPos != std::string::npos)
    {
      wxcfg.erase(univPos, univStr.length());
      return true;
    }
  }
  return false;
}

// -------------------------------------------------------------------------------------------------

/// Return true if string changed
bool ReplaceUnicode(std::string& wxcfg, bool enable)
{
  std::string::iterator lastChar = wxcfg.end() - 1;

  if(enable)
  {
    /// Pattern: Add /.*u/ if it's not already or /.*ud/ if --debug is specified
    // TODO: string::find could be better
    if(*lastChar != 'u' && *lastChar != 'd')
    {
      wxcfg += "u";
      return true;
    }
    else if(*(lastChar - 1) != 'u' && *lastChar == 'd')
    {
      *lastChar = 'u';
      wxcfg += "d";
      return true;
    }
  }
  else
  {
    /// Pattern: Remove /.*u/ if it's present or /.*ud/ if --debug is specified
    if(*lastChar == 'u')
    {
      wxcfg.erase(lastChar);
      return true;
    }
    else if(*(lastChar - 1) == 'u' && *lastChar == 'd')
    {
      wxcfg.erase(lastChar - 1);
      return true;
    }
  }
  return false;
}

// -------------------------------------------------------------------------------------------------

/// Return true if string changed
bool ReplaceDebug(std::string& wxcfg, bool enable)
{
  std::string::iterator lastChar = wxcfg.end() - 1;

  if(enable)
  {
    /// Pattern: Add /.*d/ if it's not already
    if(*lastChar != 'd')
    {
      wxcfg += "d";
      return true;
    }
  }
  else
  {
    /// Pattern: Remove /.*d/ if it's present
    if(*lastChar == 'd')
    {
      wxcfg.erase(lastChar);
      return true;
    }
  }
  return false;
}

// -------------------------------------------------------------------------------------------------

/// Return true if string changed
bool ReplaceStatic(std::string& wxcfg, bool enable)
{
  if(enable)
  {
    /// Pattern: Replace /.*_dll/ to /.*_lib/
    size_t loc = wxcfg.find("_dll");
    if(loc != std::string::npos)
    {
      wxcfg.replace(loc, std::string("_lib").length(), "_lib");
      return true;
    }
  }
  else
  {
    /// Pattern: Replace /.*_lib/ to /.*_dll/
    size_t loc = wxcfg.find("_lib");
    if(loc != std::string::npos)
    {
      wxcfg.replace(loc, std::string("_dll").length(), "_dll");
      return true;
    }
  }
  return false;
}

// -------------------------------------------------------------------------------------------------

void AutodetectConfiguration(Options& po, const CmdLineOptions& cl)
{
  // TODO: still directory listing is needed, to account for $(CFG), $(DIR_SUFFIX_CPU), ...
  typedef std::vector<std::string> VecCfgs;

  VecCfgs cfgs;
  VecCfgs newCfgs;
  std::string cfg;
  VecCfgs::iterator itCfg;

  // Iterate through the options the user didn't supply
  cfgs.push_back(po["wxcfg"]);
  if(!cl.KeyExists("--universal"))
  {
    cfg = po["wxcfg"];
    if(ReplaceUniv(cfg, true))
      cfgs.push_back(cfg);

    cfg = po["wxcfg"];
    if(ReplaceUniv(cfg, false))
      cfgs.push_back(cfg);
  }

  if(!cl.KeyExists("--unicode"))
  {
    newCfgs.clear();
    for(itCfg = cfgs.begin(); itCfg != cfgs.end(); ++itCfg)
    {
      cfg = *itCfg;
      if(ReplaceUnicode(cfg, true))
        newCfgs.push_back(cfg);

      cfg = *itCfg;
      if(ReplaceUnicode(cfg, false))
        newCfgs.push_back(cfg);
    }

    for(itCfg = newCfgs.begin(); itCfg != newCfgs.end(); ++itCfg)
      cfgs.push_back(*itCfg);
  }

  if(!cl.KeyExists("--debug"))
  {
    newCfgs.clear();
    for(itCfg = cfgs.begin(); itCfg != cfgs.end(); ++itCfg)
    {
      cfg = *itCfg;
      if(ReplaceDebug(cfg, true))
        newCfgs.push_back(cfg);

      cfg = *itCfg;
      if(ReplaceDebug(cfg, false))
        newCfgs.push_back(cfg);
    }

    for(itCfg = newCfgs.begin(); itCfg != newCfgs.end(); ++itCfg)
      cfgs.push_back(*itCfg);
  }

  if(!cl.KeyExists("--static"))
  {
    newCfgs.clear();
    for(itCfg = cfgs.begin(); itCfg != cfgs.end(); ++itCfg)
    {
      cfg = *itCfg;
      if(ReplaceStatic(cfg, true))
        newCfgs.push_back(cfg);

      cfg = *itCfg;
      if(ReplaceStatic(cfg, false))
        newCfgs.push_back(cfg);
    }

    for(itCfg = newCfgs.begin(); itCfg != newCfgs.end(); ++itCfg)
      cfgs.push_back(*itCfg);
  }

  if(!cl.KeyExists("--compiler"))
  {
    newCfgs.clear();
    for(itCfg = cfgs.begin(); itCfg != cfgs.end(); ++itCfg)
    {
      cfg = *itCfg;
      if(ReplaceCompilerIfFound(cfg, "gcc"))
        newCfgs.push_back(cfg);

      cfg = *itCfg;
      if(ReplaceCompilerIfFound(cfg, "dmc"))
        newCfgs.push_back(cfg);

      cfg = *itCfg;
      if(ReplaceCompilerIfFound(cfg, "vc"))
        newCfgs.push_back(cfg);

      cfg = *itCfg;
      if(ReplaceCompilerIfFound(cfg, "wat"))
        newCfgs.push_back(cfg);

      cfg = *itCfg;
      if(ReplaceCompilerIfFound(cfg, "bcc"))
        newCfgs.push_back(cfg);
    }

    for(itCfg = newCfgs.begin(); itCfg != newCfgs.end(); ++itCfg)
      cfgs.push_back(*itCfg);
  }

  // reads the first setup.h it founds
  bool found = false;
  for(VecCfgs::const_iterator it = cfgs.begin(); it != cfgs.end(); ++it)
  {
    std::string file = po["prefix"] + "/lib/" + *it + "/wx/setup.h";
    std::ifstream setupH(file.c_str());
    if(setupH.is_open())
    {
      if(!found)
      {
        po["wxcfg"] = *it;
        found = true;
      }
      else
      {
        std::cerr << g_tokWarning << "Multiple compiled configurations of wxWidgets have been detected." << std::endl;
        std::cerr << "Using first detected version by default." << std::endl;
        std::cerr << std::endl;
        std::cerr << "Please use the --wxcfg flag (as in wx-config --wxcfg=gcc_dll/mswud)" << std::endl;
        std::cerr << "or set the environment variable WXCFG (as in WXCFG=gcc_dll/mswud)" << std::endl;
        std::cerr << "to specify which configuration exactly you want to use." << std::endl;
        return;
      }
    }
  }

  if(!po.KeyExists("wxcfg"))
  {
    // TODO: this never reaches thanks to the new autodetection algorithm

    std::cout << g_tokError << "No setup.h file has been auto-detected." << std::endl;
    std::cerr << std::endl;
    std::cerr << "Please use the --wxcfg flag (as in wx-config --wxcfg=gcc_dll/mswud)" << std::endl;
    std::cerr << "or set the environment variable WXCFG (as in WXCFG=gcc_dll/mswud)" << std::endl;
    std::cerr << "to specify which configuration exactly you want to use." << std::endl;

    exit(1);
  }
}

// -------------------------------------------------------------------------------------------------

void CheckAdditionalFlags(Options& po, const CmdLineOptions& cl)
{
  /// Modifies wxcfg as 'vc[cpu]_[dll|lib][CFG]\[msw|base][univ][u][d]' accordingly
  /// or 'gcc_[dll|lib][CFG]\[msw|base][univ][u][d]'

  if(cl.KeyExists("--universal"))
  {
    if(cl.KeyValue("--universal") == "no")
      ReplaceUniv(po["wxcfg"], false);
    else if(cl.KeyValue("--universal") == "yes" || cl.KeyValue("--universal").empty())
      ReplaceUniv(po["wxcfg"], true);
  }

  if(cl.KeyExists("--unicode"))
  {
    if(cl.KeyValue("--unicode") == "no")
      ReplaceUnicode(po["wxcfg"], false);
    else if(cl.KeyValue("--unicode") == "yes" || cl.KeyValue("--unicode").empty())
      ReplaceUnicode(po["wxcfg"], true);
  }

  if(cl.KeyExists("--debug"))
  {
    if(cl.KeyValue("--debug") == "no")
      ReplaceDebug(po["wxcfg"], false);
    else if(cl.KeyValue("--debug") == "yes" || cl.KeyValue("--debug").empty())
      ReplaceDebug(po["wxcfg"], true);
  }

  if(cl.KeyExists("--static"))
  {
    if(cl.KeyValue("--static") == "no")
      ReplaceStatic(po["wxcfg"], false);
    else if(cl.KeyValue("--static") == "yes" || cl.KeyValue("--static").empty())
      ReplaceStatic(po["wxcfg"], true);
  }

  if(cl.KeyExists("--compiler"))
  {
    if(cl.KeyValue("--compiler") == "gcc")
      ReplaceCompilerIfFound(po["wxcfg"], "gcc");
    else if(cl.KeyValue("--compiler") == "dmc")
      ReplaceCompilerIfFound(po["wxcfg"], "dmc");
    else if(cl.KeyValue("--compiler") == "vc")
      ReplaceCompilerIfFound(po["wxcfg"], "vc");
    else if(cl.KeyValue("--compiler") == "wat")
      ReplaceCompilerIfFound(po["wxcfg"], "wat");
    else if(cl.KeyValue("--compiler") == "bcc")
      ReplaceCompilerIfFound(po["wxcfg"], "bcc");
  }
}

// -------------------------------------------------------------------------------------------------

bool DetectCompiler(Options& po, const CmdLineOptions& cl)
{
  // input example of po["wxcfg"]:
  // gcc_dll/mswud
  // vc_lib/msw
  Compiler* pCompiler = NULL;

  // if (po["wxcfg"].find("gcc_") != std::string::npos)
  if(po["wxcfg"].find("gcc") != std::string::npos)
  {
    CompilerMinGW* pcompiler = new CompilerMinGW();
    pCompiler = pcompiler;
  }
  // else if(po["wxcfg"].find("dmc_") != std::string::npos)
  else if(po["wxcfg"].find("dmc") != std::string::npos)
  {
    CompilerDMC* pcompiler = new CompilerDMC();
    pCompiler = pcompiler;
  }
  // else if(po["wxcfg"].find("vc_") != std::string::npos)
  else if(po["wxcfg"].find("vc") != std::string::npos)
  {
    CompilerVC* pcompiler = new CompilerVC();
    pCompiler = pcompiler;
  }
  // else if(po["wxcfg"].find("wat_") != std::string::npos)
  else if(po["wxcfg"].find("wat") != std::string::npos)
  {
    CompilerWAT* pcompiler = new CompilerWAT();
    pCompiler = pcompiler;
  }
  // else if(po["wxcfg"].find("bcc_") != std::string::npos)
  else if(po["wxcfg"].find("bcc") != std::string::npos)
  {
    CompilerBCC* pcompiler = new CompilerBCC();
    pCompiler = pcompiler;
  }

  if(pCompiler)
  {
    pCompiler->Process(po, cl);
    delete pCompiler;
    return true;
  }
  else
  {
    // TODO: this never reaches thanks to the new autodetection algorithm

    std::cout << g_tokError << "No supported compiler has been detected in the configuration '" << po["wxcfg"] << "'."
              << std::endl;
    std::cerr << std::endl;
    std::cerr << "The specified wxcfg must start with a 'gcc_', 'dmc_' or 'vc_'" << std::endl;
    std::cerr << "to be successfully detected." << std::endl;

    // exit(1);
  }

  return false;
}

// -------------------------------------------------------------------------------------------------

void ValidatePrefix(const std::string& prefix)
{
  // tests if prefix is a valid dir. checking if there is an \include\wx\wx.h
  const std::string testfile = prefix + "/include/wx/wx.h";
  std::ifstream prefixIsValid(testfile.c_str());
  if(!prefixIsValid.is_open())
  {
    std::cout << g_tokError << "wxWidgets hasn't been found installed at '" << prefix << "'." << std::endl;
    std::cerr << std::endl;
    std::cerr << "Please use the --prefix flag (as in wx-config --prefix=C:/wxWidgets)" << std::endl;
    std::cerr << "or set the environment variable WXWIN (as in WXWIN=C:/wxWidgets)" << std::endl;
    std::cerr << "to specify where is your installation of wxWidgets." << std::endl;

    exit(1);
  }
}

// -------------------------------------------------------------------------------------------------

bool ValidateConfiguration(const std::string& wxcfgfile, const std::string& wxcfgsetuphfile, bool exitIfError = true)
{
  std::ifstream build_cfg(wxcfgfile.c_str());
  std::ifstream setup_h(wxcfgsetuphfile.c_str());

  bool isBuildCfgOpen = build_cfg.is_open();
  bool isSetupHOpen = setup_h.is_open();

  if(exitIfError)
  {
    if(!isBuildCfgOpen)
    {
      if(!isSetupHOpen)
      {
        std::cout << g_tokError << "No valid setup.h of wxWidgets has been found at location: " << wxcfgsetuphfile
                  << std::endl;
        std::cerr << std::endl;
        std::cerr << "Please use the --wxcfg flag (as in wx-config --wxcfg=gcc_dll/mswud)" << std::endl;
        std::cerr << "or set the environment variable WXCFG (as in WXCFG=gcc_dll/mswud)" << std::endl;
        std::cerr << "to specify which configuration exactly you want to use." << std::endl;

        exit(1);
      }

      std::cout << g_tokError << "No valid configuration of wxWidgets has been found at location: " << wxcfgfile
                << std::endl;
      std::cerr << std::endl;
      std::cerr << "Please use the --wxcfg flag (as in wx-config --wxcfg=gcc_dll/mswud)" << std::endl;
      std::cerr << "or set the environment variable WXCFG (as in WXCFG=gcc_dll/mswud)" << std::endl;
      std::cerr << "to specify which configuration exactly you want to use." << std::endl;

      exit(1);
    }
  }
  return isBuildCfgOpen && isSetupHOpen;
}

// -------------------------------------------------------------------------------------------------

void OutputFlags(Options& po, const CmdLineOptions& cl)
{
  /// Outputs flags to console
  if(cl.KeyExists("--variable"))
  {
    std::cout << po["variable"];
    return;
  }

  if(cl.KeyExists("--selected-config"))
  {
    std::cout << po["selected-config"];
    return;
  }

  if(cl.KeyExists("--prefix"))
  {
    std::cout << po["prefix"];
    return;
  }

  if(cl.KeyExists("--cflags") || cl.KeyExists("--cxxflags") || cl.KeyExists("--cppflags"))
  {
    std::cout << po["cflags"] << std::endl;
    return;
  }

  if(cl.KeyExists("--libs"))
  {
    std::cout << po["libs"] << std::endl;
    return;
  }

  if(cl.KeyExists("--cc"))
    std::cout << po["cc"];
  if(cl.KeyExists("--cxx"))
    std::cout << po["cxx"];
  if(cl.KeyExists("--ld"))
    std::cout << po["ld"];
  if(cl.KeyExists("--rcflags"))
    std::cout << po["rcflags"] << std::endl;
  if(cl.KeyExists("--release"))
    std::cout << po["release"];
  if(cl.KeyExists("--version"))
    std::cout << po["version"];
  if(cl.KeyExists("--basename"))
    std::cout << po["basename"];

#if 0 // not implemented
    if (cl.KeyExists("--version=")) // incomplete
        std::cout << po["version="];
    if (cl.KeyExists("--ld")) // incomplete
        std::cout << po["ld"];
    if (cl.KeyExists("--rezflags"))
        std::cout << po["rezflags"];
    if (cl.KeyExists("--linkdeps"))
        std::cout << po["linkdeps"];
    if (cl.KeyExists("--version-full"))
        std::cout << po["version-full"];
    if (cl.KeyExists("--exec-prefix="))
        std::cout << po["exec-prefix="];
    if (cl.KeyExists("--toolkit"))
        std::cout << po["toolkit"];
    if (cl.KeyExists("--list"))
        std::cout << po["list"];
#endif
}

// -------------------------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
  /// Command line options
  CmdLineOptions cl(argc, argv);

  if(!cl.ValidArgs())
  {
    return 1;
  }

  if(cl.KeyExists("-v"))
  {
    std::cout << "wx-config revision " << GetSvnRevision() << " " << GetSvnDate() << std::endl;
    return 0;
  }

  /// Program options
  Options po;

  CheckEasyMode(cl);

  if(cl.KeyExists("--prefix") && cl["--prefix"].size()) // Exists and not empty
  {
    po["prefix"] = cl["--prefix"];
  }
  else if(getenv("WXWIN"))
  {
    std::string wx_win = getenv("WXWIN");
    std::replace(wx_win.begin(), wx_win.end(), '\\', '/');
    po["prefix"] = wx_win;
  }
  else
  {
#ifdef _WIN32
    /// Assume that, like a *nix, we're installed in C:\some\path\bin,
    /// and that the root dir of wxWidgets is therefore C:\some\path.
    DWORD length = MAX_PATH;
    LPSTR libPath = NULL;
    LPSTR filePart;

    /// Why, oh why, doesn't M$ give us a way to find the actual return length,
    /// like they do with everything else?!?
    do
    {
      libPath = new CHAR[length];
      GetModuleFileName(NULL, libPath, length);
      GetFullPathName(libPath, length, libPath, &filePart);
      *filePart = '\0';
      // Add 3 for ".." and NULL
      if(strlen(libPath) + 3 > length)
      {
        length *= 2;
        delete[] libPath;
        libPath = NULL;
      }
    } while(libPath == NULL);
    strcpy(filePart, "..");

    // Fix the ..
    GetFullPathName(libPath, length, libPath, NULL);
    po["prefix"] = libPath;
    delete[] libPath;
#else
    po["prefix"] = "C:/wxWidgets";
#endif
  }

  NormalizePath(po["prefix"]);

  ValidatePrefix(po["prefix"]);

  if(cl.KeyExists("--wxcfg") && cl["--wxcfg"].size()) // Exists and not empty
    po["wxcfg"] = cl["--wxcfg"];
  else if(getenv("WXCFG"))
  {
    std::string wx_cfg = getenv("WXCFG");
    std::replace(wx_cfg.begin(), wx_cfg.end(), '\\', '/');
    po["wxcfg"] = wx_cfg;
  }
  else
  {
    // Try if something valid can be found trough deriving checkAdditionalFlags() first
    po["wxcfg"] = "gcc_dll/msw";

    po["wxcfgfile"] = po["prefix"] + "/lib/" + po["wxcfg"] + "/build.cfg";
    po["wxcfgsetuphfile"] = po["prefix"] + "/lib/" + po["wxcfg"] + "/wx/setup.h";
    CheckAdditionalFlags(po, cl);

    if(!ValidateConfiguration(po["wxcfgfile"], po["wxcfgsetuphfile"], false))
      AutodetectConfiguration(po, cl); // important function
  }

  NormalizePath(po["wxcfg"]);
  CheckAdditionalFlags(po, cl);
  po["wxcfgfile"] = po["prefix"] + "/lib/" + po["wxcfg"] + "/build.cfg";
  po["wxcfgsetuphfile"] = po["prefix"] + "/lib/" + po["wxcfg"] + "/wx/setup.h";

  ValidateConfiguration(po["wxcfgfile"], po["wxcfgsetuphfile"]);

  if(!DetectCompiler(po, cl))
  {
    return 1;
  }

  OutputFlags(po, cl);

  return 0;
}
