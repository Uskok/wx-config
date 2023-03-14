
#include "Compiler.h"

// -------------------------------------------------------------------------------------------------

void Compiler::ProcessStart(Options& po, const CmdLineOptions& cl, BuildFileOptions& cfg, SetupHOptions& sho)
{
  std::string config_XXX = po["prefix"] + "/build/msw/config." + GetName(); /// XXX is bcc, gcc, vc or wat
  /// config.* options
  cfg.Parse(config_XXX, true);

  /// Overriding flags
  /// build.cfg options -> config.*
  cfg.Parse(po["wxcfgfile"]);

  /// setup.h options
  sho.Parse(po["wxcfgsetuphfile"]);
}

void Compiler::ProcessMain(Options& po, const CmdLineOptions& cl, BuildFileOptions& cfg, SetupHOptions& sho)
{
  // FIXME: proper place of this would be in a first hook, say process_1();
  if(cl.KeyExists("--define-variable"))
  {
    std::string strDef = cl.KeyValue("--define-variable");
    size_t sep = strDef.find("=");
    if(sep != std::string::npos)
    {
      std::string key = strDef.substr(0, sep);
      std::string val = strDef.substr(sep + 1, strDef.size() - sep - 1);

      po[key] = val;
      cfg[key] = val;

      if(val == "1" || val == "true")
        sho[key] = true;
      else if(val == "0" || val == "false")
        sho[key] = false;
    }
    else
    {
      std::cout << g_tokError << "Failed to define a variable as '" << cl.KeyValue("--define-variable") << "'."
                << std::endl;
      std::cout << "The syntax is --define-variable=VARIABLENAME=VARIABLEVALUE" << std::endl;
      exit(1);
    }
  }

  /// Overriding flags sho -> cfg !!
  /// This makes sho variables haves more privilege than cfg ones
  //-------------------------------------------------------------
  if(sho.KeyExists("wxUSE_UNICODE_MSLU"))
    sho["wxUSE_UNICODE_MSLU"] ? cfg["MSLU"] = "1" : cfg["MSLU"] = "0";

  // TODO: probably better!!!:
  if(cfg.KeyExists("MSLU"))
    sho["wxUSE_UNICODE_MSLU"] ? cfg["MSLU"] = "1" : cfg["MSLU"] = "0";

  //-------------------------------------------------------------

  /// Overriding compiler programs
  if(cfg.KeyExists("CC"))
    m_programs.cc = cfg["CC"];

  if(cfg.KeyExists("CXX"))
    m_programs.cxx = cfg["CXX"];

  if(cfg.KeyExists("LD"))
    m_programs.ld = cfg["LD"];

  if(cfg.KeyExists("LIB"))
    m_programs.lib = cfg["LIB"];

  if(cfg.KeyExists("WINDRES"))
    m_programs.windres = cfg["WINDRES"];

  //-------------------------------------------------------------

  // BASENAME variables
  po["LIB_BASENAME_MSW"] = "wx" + po["PORTNAME"] + po["WXUNIVNAME"] + po["WX_RELEASE_NODOT"];
  po["LIB_BASENAME_MSW"] += po["WXUNICODEFLAG"] + po["WXDEBUGFLAG"] + cfg["WX_LIB_FLAVOUR"];

  po["LIB_BASENAME_BASE"] = "wxbase" + po["WX_RELEASE_NODOT"] + po["WXUNICODEFLAG"];
  po["LIB_BASENAME_BASE"] += po["WXDEBUGFLAG"] + cfg["WX_LIB_FLAVOUR"];

  CmdLineOptions& clx = (CmdLineOptions&)cl;
  VecLibs& libs = clx.GetLibs();

  for(size_t i = 0; i < libs.size(); ++i)
  {
    std::string lib = libs[i];

    if(lib == "base")
    {
      if(cfg["MONOLITHIC"] == "0")
        po["__WXLIB_BASE_p"] = AddLib(po["LIB_BASENAME_BASE"]);
    }
    else if(lib == "net")
    {
      if(cfg["MONOLITHIC"] == "0")
        po["__WXLIB_NET_p"] = AddLib(po["LIB_BASENAME_BASE"] + "_net");
    }
    else if(lib == "xml")
    {
      if(cfg["MONOLITHIC"] == "0")
        po["__WXLIB_XML_p"] = AddLib(po["LIB_BASENAME_BASE"] + "_xml");
    }
    else if(lib == "core")
    {
      if(cfg["MONOLITHIC"] == "0")
        if(cfg["USE_GUI"] == "1")
          po["__WXLIB_CORE_p"] = AddLib(po["LIB_BASENAME_MSW"] + "_core");
    }
    else if(lib == "adv")
    {
      if(cfg["MONOLITHIC"] == "0")
        if(cfg["USE_GUI"] == "1")
          po["__WXLIB_ADV_p"] = AddLib(po["LIB_BASENAME_MSW"] + "_adv");
    }
    else if(lib == "rich" || lib == "richtext")
    {
      if(cfg["MONOLITHIC"] == "0")
        if(cfg["USE_GUI"] == "1")
          po["__WXLIB_RICH_p"] = AddLib(po["LIB_BASENAME_MSW"] + "_richtext");
    }
    else if(lib == "qa")
    {
      if(cfg["MONOLITHIC"] == "0")
      {
        if(cfg["USE_GUI"] == "1")
        {
          if(cfg["USE_QA"] == "1")
          {
            po["__WXLIB_QA_p"] = AddLib(po["LIB_BASENAME_MSW"] + "_qa");
            po["__WXLIB_CORE_p"] = AddLib(po["LIB_BASENAME_MSW"] + "_core");
            po["__WXLIB_XML_p"] = AddLib(po["LIB_BASENAME_BASE"] + "_xml");
          }
        }
      }
    }
    else if(lib == "xrc")
    {
      if(cfg["MONOLITHIC"] == "0")
      {
        if(cfg["USE_GUI"] == "1")
        {
          po["__WXLIB_XRC_p"] = AddLib(po["LIB_BASENAME_MSW"] + "_xrc");
          po["__WXLIB_XML_p"] = AddLib(po["LIB_BASENAME_BASE"] + "_xml");
          po["__WXLIB_ADV_p"] = AddLib(po["LIB_BASENAME_MSW"] + "_adv");
          po["__WXLIB_HTML_p"] = AddLib(po["LIB_BASENAME_MSW"] + "_html");
        }
      }
    }
    else if(lib == "aui")
    {
      if(cfg["MONOLITHIC"] == "0")
        if(cfg["USE_GUI"] == "1")
          po["__WXLIB_AUI_p"] = AddLib(po["LIB_BASENAME_MSW"] + "_aui");
    }
    else if(lib == "html")
    {
      if(cfg["MONOLITHIC"] == "0")
        if(cfg["USE_GUI"] == "1")
          po["__WXLIB_HTML_p"] = AddLib(po["LIB_BASENAME_MSW"] + "_html");
    }
    else if(lib == "media")
    {
      if(cfg["MONOLITHIC"] == "0")
        if(cfg["USE_GUI"] == "1")
          po["__WXLIB_MEDIA_p"] = AddLib(po["LIB_BASENAME_MSW"] + "_media");
    }
    else if(lib == "odbc")
    {
      if(cfg["MONOLITHIC"] == "0")
        if(sho["wxUSE_ODBC"])
          po["__WXLIB_ODBC_p"] = AddLib(po["LIB_BASENAME_BASE"] + "_odbc");
    }
    else if(lib == "dbgrid")
    {
      if(cfg["MONOLITHIC"] == "0")
      {
        if(cfg["USE_GUI"] == "1")
        {
          if(sho["wxUSE_ODBC"])
          {
            po["__WXLIB_DBGRID_p"] = AddLib(po["LIB_BASENAME_MSW"] + "_dbgrid");
            po["__WXLIB_ODBC_p"] = AddLib(po["LIB_BASENAME_BASE"] + "_odbc");
            po["__WXLIB_ADV_p"] = AddLib(po["LIB_BASENAME_MSW"] + "_adv");
          }
        }
      }
    }
    else if(lib == "webview")
    {
      if(cfg["MONOLITHIC"] == "0")
        if(cfg["USE_GUI"] == "1")
          if(cfg["USE_WEBVIEW"] == "1")
          {
            po["__WXLIB_WEBVIEW_p"] = AddLib(po["LIB_BASENAME_MSW"] + "_webview");
          }
    }
    else if(lib == "stc")
    {
      if(cfg["MONOLITHIC"] == "0")
        if(cfg["USE_GUI"] == "1")
          if(cfg["USE_STC"] == "1")
          {
            po["__WXLIB_STC_p"] = AddLib(po["LIB_BASENAME_MSW"] + "_stc");
          }
    }
    else if(lib == "propgrid")
    {
      if(cfg["MONOLITHIC"] == "0")
        if(cfg["USE_GUI"] == "1")
          if(cfg["USE_PROPGRID"] == "1")
          {
            po["__WXLIB_PROPGRID_p"] = AddLib(po["LIB_BASENAME_MSW"] + "_propgrid");
          }
    }
    else if(lib == "ribbon")
    {
      if(cfg["MONOLITHIC"] == "0")
        if(cfg["USE_GUI"] == "1")
          if(cfg["USE_RIBBON"] == "1")
          {
            po["__WXLIB_RIBBON_p"] = AddLib(po["LIB_BASENAME_MSW"] + "_ribbon");
          }
    }
    else if(lib == "opengl" || lib == "gl")
    {
      if(cfg["USE_OPENGL"] == "1")
        if(cfg["USE_GUI"] == "1")
          // TODO: it's opengl or gl?
          /// Doesn't matter if it's monolithic or not
          po["__WXLIB_OPENGL_p"] = AddLib(po["LIB_BASENAME_MSW"] + "_gl");
      po["__WXLIB_OPENGL_p"] += AddLib("opengl32");
      po["__WXLIB_OPENGL_p"] += AddLib("glu32");
    }
    else
    {
      /// Doesn't matter if it's monolithic or not
      po["__WXLIB_ARGS_p"] += AddLib(po["LIB_BASENAME_MSW"] + "_" + lib);
    }
  }

  if(cfg["MONOLITHIC"] == "1")
    po["__WXLIB_MONO_p"] = AddLib(po["LIB_BASENAME_MSW"]);

  /// External libs (to wxWidgets)

  if(cfg["USE_GUI"] == "1")
    if(cfg["USE_STC"] == "1")
      po["__LIB_WXSCINTILLA_p"] = AddLib("wxscintilla" + po["WXDEBUGFLAG"]);

  if(cfg["USE_GUI"] == "1")
    if(sho["wxUSE_LIBTIFF"])
      po["__LIB_TIFF_p"] = AddLib("wxtiff" + po["WXDEBUGFLAG"]);

  if(cfg["USE_GUI"] == "1")
    if(sho["wxUSE_LIBJPEG"])
      po["__LIB_JPEG_p"] = AddLib("wxjpeg" + po["WXDEBUGFLAG"]);

  if(cfg["USE_GUI"] == "1")
    if(sho["wxUSE_LIBPNG"] && sho["wxUSE_ZLIB"])
      po["__LIB_PNG_p"] = AddLib("wxpng" + po["WXDEBUGFLAG"]);

  if(sho["wxUSE_ZLIB"])
    po["__LIB_ZLIB_p"] = AddLib("wxzlib" + po["WXDEBUGFLAG"]);

  if(sho["wxUSE_REGEX"])
    po["__LIB_REGEX_p"] = AddLib("wxregex" + po["WXUNICODEFLAG"] + po["WXDEBUGFLAG"]);

  // FIXME: in truth the check should be for wxUSE_XML but... the sho parser is very simple :P
  if(sho["wxUSE_XRC"])
    po["__LIB_EXPAT_p"] = AddLib("wxexpat" + po["WXDEBUGFLAG"]);

  if(cfg["MSLU"] == "1")
    po["__LIB_UNICOWS_p"] = AddLib("unicows");

  if(cfg["USE_GDIPLUS"] == "1")
    po["__GDIPLUS_LIB_p"] = AddLib("gdiplus");

  po["__LIB_KERNEL32_p"] = AddLib("kernel32");

  po["__LIB_USER32_p"] = AddLib("user32");

  po["__LIB_GDI32_p"] = AddLib("gdi32");

  po["__LIB_COMDLG32_p"] = AddLib("comdlg32");

  po["__LIB_WINSPOOL_p"] = AddLib("winspool");

  po["__LIB_WINMM_p"] = AddLib("winmm");

  po["__LIB_SHELL32_p"] = AddLib("shell32");

  po["__LIB_COMCTL32_p"] = AddLib("comctl32");

  po["__LIB_VERSION_p"] = AddLib("version");

  po["__LIB_SHLWAPI_p"] = AddLib("shlwapi");

  if(sho["wxUSE_OLE"])
    po["__LIB_OLE32_p"] = AddLib("ole32");

  if(sho["wxUSE_OLE"])
    po["__LIB_OLEAUT32_p"] = AddLib("oleaut32");

  if(sho["wxUSE_OLE"])
    po["__LIB_OLEACC_p"] = AddLib("oleacc"); // NOTE: not being used

  if(sho["wxUSE_OLE"])
    po["__LIB_OLE2W32_p"] = AddLib("ole2w32"); // NOTE: not being used

  po["__LIB_UUID_p"] = AddLib("uuid");

  po["__LIB_RPCRT4_p"] = AddLib("rpcrt4");

  po["__LIB_ADVAPI32_p"] = AddLib("advapi32");

  if(sho["wxUSE_SOCKETS"])
    po["__LIB_WSOCK32_p"] = AddLib("wsock32");

  if(sho["wxUSE_ODBC"])
    po["__LIB_ODBC32_p"] = AddLib("odbc32");

  /*      TODO: From BAKEFILE
          <!-- link-in system libs that wx depends on: -->
          <!-- If on borland, we don't need to do much            -->
          <if cond="FORMAT=='borland'">
              <sys-lib>ole2w32</sys-lib>
              <sys-lib>odbc32</sys-lib>
          </if>

          <!-- Non-borland, on the other hand...                  -->
          <if cond="FORMAT not in ['borland','msevc4prj']">
              <sys-lib>kernel32</sys-lib>
              <sys-lib>user32</sys-lib>
              <sys-lib>gdi32</sys-lib>
              <sys-lib>comdlg32</sys-lib>
              <sys-lib>winspool</sys-lib>
              <sys-lib>winmm</sys-lib>
              <sys-lib>shell32</sys-lib>
              <sys-lib>comctl32</sys-lib>
              <sys-lib>version</sys-lib>
              <sys-lib>shlwapi</sys-lib>
              <sys-lib>ole32</sys-lib>
              <sys-lib>oleaut32</sys-lib>
              <sys-lib>uuid</sys-lib>
              <sys-lib>rpcrt4</sys-lib>
              <sys-lib>advapi32</sys-lib>
              <sys-lib>wsock32</sys-lib>
              <sys-lib>odbc32</sys-lib>
          </if>

          <!-- Libs common to both borland and MSVC               -->
          <if cond="FORMAT=='msvc' or FORMAT=='msvc6prj' or FORMAT=='borland'">
              <sys-lib>oleacc</sys-lib>
  */
}

void Compiler::ProcessEnd(Options& po, const CmdLineOptions& cl, BuildFileOptions& cfg)
{
  //"msw-unicode-3.0"
  po["selected-config"] = po["PORTNAME"] + "-"; // msw
  if(cfg["UNICODE"] == "1")
    po["selected-config"] += "unicode-";  // "unicode"
  po["selected-config"] += po["release"]; // "3.0"

  // or "mswu"
  //        po["selected-config"] = po["PORTNAME"]; //msw
  //        if (cfg["UNICODE"] == "1")
  //          po["selected-config"] += "u"; // "unicode"
}

// -------------------------------------------------------------------------------------------------

void CompilerMinGW::Process(Options& po, const CmdLineOptions& cl)
{
  /// Searchs for '<prefix>\build\msw\config.*' first
  // std::string cfg_first = po["prefix"] + "/build/msw/config." + GetName();

  /// config.* options
  // BuildFileOptions cfg(cfg_first);
  BuildFileOptions cfg;
  SetupHOptions sho;

  Compiler::ProcessStart(po, cl, cfg, sho);

  // ### Variables: ###
  po["WX_RELEASE_NODOT"] = cfg["WXVER_MAJOR"] + cfg["WXVER_MINOR"];
  if(po["WX_RELEASE_NODOT"].empty())
    po["WX_RELEASE_NODOT"] = "26";

  // ### Conditionally set variables: ###
  if(cfg["GCC_VERSION"] == "2.95")
    po["GCCFLAGS"] = AddFlag("-fvtable-thunks");

  if(cfg["USE_GUI"] == "0")
    po["PORTNAME"] = "base";

  if(cfg["USE_GUI"] == "1")
    po["PORTNAME"] = "msw";

  //        if (cfg["DEBUG_FLAG"] == "1")
  //            po["WXDEBUGFLAG"] = "d";

  if(cfg["BUILD"] == "debug")
    po["WXDEBUGFLAG"] = "d";

  if(cfg["UNICODE"] == "1")
    po["WXUNICODEFLAG"] = "u";

  if(cfg["WXUNIV"] == "1")
    po["WXUNIVNAME"] = "univ";

  if(cfg["SHARED"] == "1")
    po["WXDLLFLAG"] = "dll";

  if(cfg["SHARED"] == "0")
    po["LIBTYPE_SUFFIX"] = "lib";

  if(cfg["SHARED"] == "1")
    po["LIBTYPE_SUFFIX"] = "dll";

  if(cfg["MONOLITHIC"] == "0")
    po["EXTRALIBS_FOR_BASE"] = "";

  if(cfg["MONOLITHIC"] == "1")
    po["EXTRALIBS_FOR_BASE"] = "";

  if(cfg["BUILD"] == "debug")
    po["__OPTIMIZEFLAG_2"] = AddFlag("-O0");

  if(cfg["BUILD"] == "release")
    po["__OPTIMIZEFLAG_2"] = AddFlag("-O2");

  if(cfg["USE_RTTI"] == "1")
    po["__RTTIFLAG_5"] = AddFlag("");

  if(cfg["USE_EXCEPTIONS"] == "0")
    po["__EXCEPTIONSFLAG_6"] = AddFlag("-fno-exceptions");

  if(cfg["USE_EXCEPTIONS"] == "1")
    po["__EXCEPTIONSFLAG_6"] = AddFlag("");

  if(cfg["WXUNIV"] == "1")
    po["__WXUNIV_DEFINE_p"] = AddDefine("__WXUNIVERSAL__");

  if(cfg["WXUNIV"] == "1")
    po["__WXUNIV_DEFINE_p_1"] = AddResDefine("__WXUNIVERSAL__");

  if(cfg["BUILD"] == "debug")
    po["__DEBUG_DEFINE_p"] = AddDefine("__WXDEBUG__");

  //        if (cfg["DEBUG_FLAG"] == "1")
  //            po["__DEBUG_DEFINE_p"] = AddDefine("__WXDEBUG__");

  if(cfg["BUILD"] == "debug")
    po["__DEBUG_DEFINE_p_1"] = AddResDefine("__WXDEBUG__");

  //        if (cfg["DEBUG_FLAG"] == "1")
  //            po["__DEBUG_DEFINE_p_1"] = AddResDefine("__WXDEBUG__");

  if(cfg["USE_EXCEPTIONS"] == "0")
    po["__EXCEPTIONS_DEFINE_p"] = AddDefine("wxNO_EXCEPTIONS");

  if(cfg["USE_EXCEPTIONS"] == "0")
    po["__EXCEPTIONS_DEFINE_p_1"] = AddResDefine("wxNO_EXCEPTIONS");

  if(cfg["USE_RTTI"] == "0")
    po["__RTTI_DEFINE_p"] = AddDefine("wxNO_RTTI");

  if(cfg["USE_RTTI"] == "0")
    po["__RTTI_DEFINE_p_1"] = AddResDefine("wxNO_RTTI");

  if(cfg["USE_THREADS"] == "0")
    po["__THREAD_DEFINE_p"] = AddDefine("wxNO_THREADS");

  if(cfg["USE_THREADS"] == "0")
    po["__THREAD_DEFINE_p_1"] = AddResDefine("wxNO_THREADS");

  if(cfg["UNICODE"] == "1")
    po["__UNICODE_DEFINE_p"] = AddDefine("_UNICODE");

  if(cfg["UNICODE"] == "1")
    po["__UNICODE_DEFINE_p_1"] = AddResDefine("_UNICODE");

  if(cfg["MSLU"] == "1")
    po["__MSLU_DEFINE_p"] = AddDefine("wxUSE_UNICODE_MSLU=1");

  if(cfg["MSLU"] == "1")
    po["__MSLU_DEFINE_p_1"] = AddResDefine("wxUSE_UNICODE_MSLU=1");

  if(cfg["USE_GDIPLUS"] == "1")
    po["__GFXCTX_DEFINE_p"] = AddDefine("wxUSE_GRAPHICS_CONTEXT=1");

  if(cfg["USE_GDIPLUS"] == "1")
    po["__GFXCTX_DEFINE_p_1"] = AddResDefine("wxUSE_GRAPHICS_CONTEXT=1");

  if(cfg["SHARED"] == "1")
    po["__DLLFLAG_p"] = AddDefine("WXUSINGDLL");

  if(cfg["SHARED"] == "1")
    po["__DLLFLAG_p_1"] = AddResDefine("WXUSINGDLL");

  Compiler::ProcessMain(po, cl, cfg, sho);

  //----------------------------------------------------

  if(cfg["BUILD"] == "debug" && cfg["DEBUG_INFO"] == "default")
    po["__DEBUGINFO"] = AddFlag("-g");

  if(cfg["BUILD"] == "release" && cfg["DEBUG_INFO"] == "default")
    po["__DEBUGINFO"] = AddFlag("");

  if(cfg["DEBUG_INFO"] == "0")
    po["__DEBUGINFO"] = AddFlag("");

  if(cfg["DEBUG_INFO"] == "1")
    po["__DEBUGINFO"] = AddFlag("-g");

  if(cfg["USE_THREADS"] == "0")
    po["__THREADSFLAG"] = AddFlag("");

  if(cfg["USE_THREADS"] == "1")
    po["__THREADSFLAG"] = AddFlag("-mthreads");

  //----------------------------------------------------

  // ### Variables, Part 2: ###
  // po["LIBDIRNAME"] = po["prefix"] + "/lib/" + GetName() + "_" + po["LIBTYPE_SUFFIX"] + cfg["CFG"];
  po["LIBDIRNAME"] =
      po["prefix"] + "/lib/" + cfg["COMPILER"] + cfg["COMPILER_VERSION"] + "_" + po["LIBTYPE_SUFFIX"] + cfg["CFG"];

  po["SETUPHDIR"] = po["LIBDIRNAME"] + "/" + po["PORTNAME"] + po["WXUNIVNAME"];
  po["SETUPHDIR"] += po["WXUNICODEFLAG"] + po["WXDEBUGFLAG"];

  po["cflags"] = EasyMode(po["__DEBUGINFO"]) + EasyMode(po["__OPTIMIZEFLAG_2"]) + po["__THREADSFLAG"];
  po["cflags"] += po["GCCFLAGS"] + AddDefine("HAVE_W32API_H") + AddDefine("__WXMSW__") + po["__WXUNIV_DEFINE_p"];
  if(cfg["BUILD"] == "release")
    po["cflags"] += AddDefine("NDEBUG");

  po["cflags"] += po["__DEBUG_DEFINE_p"] + po["__EXCEPTIONS_DEFINE_p"] + po["__RTTI_DEFINE_p"];
  po["cflags"] += po["__THREAD_DEFINE_p"] + po["__UNICODE_DEFINE_p"] + po["__MSLU_DEFINE_p"];
  po["cflags"] += po["__GFXCTX_DEFINE_p"];
  po["cflags"] += AddIncludeDir(po["SETUPHDIR"]) + AddIncludeDir(po["prefix"] + "/include"); /*-W */
  po["cflags"] += EasyMode(AddFlag("-Wall")) + EasyMode(AddIncludeDir(".")) + po["__DLLFLAG_p"];
  po["cflags"] += EasyMode(AddIncludeDir("./../../samples")) + /*AddDefine("NOPCH") +*/ po["__RTTIFLAG_5"] +
                  po["__EXCEPTIONSFLAG_6"];
  po["cflags"] += AddFlag("-Wno-ctor-dtor-privacy") + AddFlag("-pipe") + AddFlag("-fmessage-length=0");
  po["cflags"] += cfg["CPPFLAGS"] + " " + cfg["CXXFLAGS"] + " ";

  po["libs"] = cfg["LDFLAGS"] + " ";
  po["libs"] += EasyMode(po["__DEBUGINFO"]) + po["__THREADSFLAG"];
  po["libs"] += AddLinkerDir(po["LIBDIRNAME"]);
  po["libs"] += EasyMode(AddFlag("-Wl,--subsystem,windows")) + EasyMode(AddFlag("-mwindows"));
  po["libs"] += GetAllLibs(po);

  /*
  po["libs"] += po["__WXLIB_ARGS_p"] + po["__WXLIB_OPENGL_p"] + po["__WXLIB_MEDIA_p"];
  po["libs"] += po["__WXLIB_DBGRID_p"] + po["__WXLIB_ODBC_p"] + po["__WXLIB_XRC_p"];
  po["libs"] += po["__WXLIB_QA_p"] + po["__WXLIB_AUI_p"] + po["__WXLIB_HTML_p"] + po["__WXLIB_ADV_p"];
  po["libs"] += po["__WXLIB_CORE_p"] + po["__WXLIB_XML_p"] + po["__WXLIB_NET_p"];
  po["libs"] += po["__WXLIB_BASE_p"] + po["__WXLIB_MONO_p"];
  po["libs"] += po["__LIB_TIFF_p"] + po["__LIB_JPEG_p"] + po["__LIB_PNG_p"];
  po["libs"] += addLib("wxzlib" + po["WXDEBUGFLAG"]) + addLib("wxregex" + po["WXUNICODEFLAG"] + po["WXDEBUGFLAG"]);
  po["libs"] += addLib("wxexpat" + po["WXDEBUGFLAG"]) + po["EXTRALIBS_FOR_BASE"] + po["__UNICOWS_LIB_p"];
  po["libs"] += addLib("kernel32") + addLib("user32") + addLib("gdi32") + addLib("comdlg32") + addLib("winspool");
  po["libs"] += addLib("winmm") + addLib("shell32") + addLib("comctl32") + addLib("ole32") + addLib("oleaut32");
  po["libs"] += addLib("uuid") + addLib("rpcrt4") + addLib("advapi32") + addLib("wsock32") + addLib("odbc32");
  */
  po["rcflags"] = AddFlag("--use-temp-file") + AddResDefine("__WXMSW__") + po["__WXUNIV_DEFINE_p_1"];
  po["rcflags"] += po["__DEBUG_DEFINE_p_1"] + po["__EXCEPTIONS_DEFINE_p_1"];
  po["rcflags"] += po["__RTTI_DEFINE_p_1"] + po["__THREAD_DEFINE_p_1"] + po["__UNICODE_DEFINE_p_1"];
  po["rcflags"] += po["__MSLU_DEFINE_p_1"] + po["__GFXCTX_DEFINE_p_1"] + AddResIncludeDir(po["SETUPHDIR"]);
  po["rcflags"] += AddResIncludeDir(po["prefix"] + "/include") + EasyMode(AddResIncludeDir("."));
  po["rcflags"] += po["__DLLFLAG_p_1"];
  po["rcflags"] += EasyMode(AddResIncludeDir(po["prefix"] + "/samples"));

  po["release"] = cfg["WXVER_MAJOR"] + "." + cfg["WXVER_MINOR"];
  po["version"] = cfg["WXVER_MAJOR"] + "." + cfg["WXVER_MINOR"] + "." + cfg["WXVER_RELEASE"];
  po["basename"] = "wx" + po["PORTNAME"] + po["WXUNIVNAME"] + po["WX_RELEASE_NODOT"];
  po["basename"] += po["WXUNICODEFLAG"] + po["WXDEBUGFLAG"] + cfg["WX_LIB_FLAVOUR"];
  po["cc"] = m_programs.cc;
  po["cxx"] = m_programs.cxx;
  po["ld"] = m_programs.ld;

  GetVariablesValues(po, cl, cfg);

  ProcessEnd(po, cl, cfg);
}

// -------------------------------------------------------------------------------------------------

void CompilerVC::Process(Options& po, const CmdLineOptions& cl)
{
  /// Searchs for '<prefix>\build\msw\config.*' first
  // std::string cfg_first = po["prefix"] + "/build/msw/config." + GetName();

  /// config.* options
  // BuildFileOptions cfg(cfg_first);
  BuildFileOptions cfg;
  SetupHOptions sho;

  Compiler::ProcessStart(po, cl, cfg, sho);

  // ### Variables: ###
  po["WX_RELEASE_NODOT"] = cfg["WXVER_MAJOR"] + cfg["WXVER_MINOR"];
  if(po["WX_RELEASE_NODOT"].empty())
    po["WX_RELEASE_NODOT"] = "26";

  // ### Conditionally set variables: ###
  if(cfg["USE_GUI"] == "0")
    po["PORTNAME"] = "base";

  if(cfg["USE_GUI"] == "1")
    po["PORTNAME"] = "msw";

  if(cfg["BUILD"] == "debug")
    po["WXDEBUGFLAG"] = "d";

  //        if (cfg["DEBUG_FLAG"] == "1")
  //            po["WXDEBUGFLAG"] = "d";

  if(cfg["UNICODE"] == "1")
    po["WXUNICODEFLAG"] = "u";

  if(cfg["WXUNIV"] == "1")
    po["WXUNIVNAME"] = "univ";

  if(cfg["SHARED"] == "1")
    po["WXDLLFLAG"] = "dll";

  if(cfg["SHARED"] == "0")
    po["LIBTYPE_SUFFIX"] = "lib";

  if(cfg["SHARED"] == "1")
    po["LIBTYPE_SUFFIX"] = "dll";

  if(cfg["MONOLITHIC"] == "0")
    po["EXTRALIBS_FOR_BASE"] = "";

  if(cfg["MONOLITHIC"] == "1")
    po["EXTRALIBS_FOR_BASE"] = "";

  if(cfg["TARGET_CPU"] == "amd64")
    po["DIR_SUFFIX_CPU"] = "_amd64";

  if(cfg["TARGET_CPU"] == "ia64")
    po["DIR_SUFFIX_CPU"] = "_ia64";

  // TODO: TARGET_CPU uses $(CPU), so it will be ignored unless explicitely specified
  if(cfg["TARGET_CPU"] == "$(CPU)")
    po["LINK_TARGET_CPU"] = "";

  if(cfg["TARGET_CPU"] == "amd64")
    po["LINK_TARGET_CPU"] = AddFlag("/MACHINE:AMD64");

  if(cfg["TARGET_CPU"] == "ia64")
    po["LINK_TARGET_CPU"] = AddFlag("/MACHINE:IA64");

  //----------------------------------------------------

  if(cfg["BUILD"] == "debug" && cfg["DEBUG_INFO"] == "default")
    po["__DEBUGINFO_0"] = AddFlag("/Zi");

  if(cfg["BUILD"] == "release" && cfg["DEBUG_INFO"] == "default")
    po["__DEBUGINFO_0"] = AddFlag("");

  if(cfg["DEBUG_INFO"] == "0")
    po["__DEBUGINFO_0"] = AddFlag("");

  if(cfg["DEBUG_INFO"] == "1")
    po["__DEBUGINFO_0"] = AddFlag("/Zi");

  if(cfg["BUILD"] == "debug" && cfg["DEBUG_INFO"] == "default")
    po["__DEBUGINFO_1"] = AddFlag("/DEBUG");

  if(cfg["BUILD"] == "release" && cfg["DEBUG_INFO"] == "default")
    po["__DEBUGINFO_1"] = AddFlag("");

  if(cfg["DEBUG_INFO"] == "0")
    po["__DEBUGINFO_1"] = AddFlag("");

  if(cfg["DEBUG_INFO"] == "1")
    po["__DEBUGINFO_1"] = AddFlag("/DEBUG");

  if(cfg["BUILD"] == "debug" && cfg["DEBUG_RUNTIME_LIBS"] == "default")
    po["____DEBUGRUNTIME_2_p"] = AddDefine("_DEBUG");

  if(cfg["BUILD"] == "release" && cfg["DEBUG_RUNTIME_LIBS"] == "default")
    po["____DEBUGRUNTIME_2_p"] = AddDefine("");

  if(cfg["DEBUG_RUNTIME_LIBS"] == "0")
    po["____DEBUGRUNTIME_2_p"] = AddDefine("");

  if(cfg["DEBUG_RUNTIME_LIBS"] == "1")
    po["____DEBUGRUNTIME_2_p"] = AddDefine("_DEBUG");

  if(cfg["BUILD"] == "debug" && cfg["DEBUG_RUNTIME_LIBS"] == "default")
    po["____DEBUGRUNTIME_2_p_1"] = AddResDefine("_DEBUG");

  if(cfg["BUILD"] == "release" && cfg["DEBUG_RUNTIME_LIBS"] == "default")
    po["____DEBUGRUNTIME_2_p_1"] = AddResDefine("");

  if(cfg["DEBUG_RUNTIME_LIBS"] == "0")
    po["____DEBUGRUNTIME_2_p_1"] = AddResDefine("");

  if(cfg["DEBUG_RUNTIME_LIBS"] == "1")
    po["____DEBUGRUNTIME_2_p_1"] = AddResDefine("_DEBUG");

  if(cfg["BUILD"] == "debug" && cfg["DEBUG_RUNTIME_LIBS"] == "default")
    po["__DEBUGRUNTIME_3"] = "d";

  if(cfg["BUILD"] == "release" && cfg["DEBUG_RUNTIME_LIBS"] == "default")
    po["__DEBUGRUNTIME_3"] = "";

  if(cfg["DEBUG_RUNTIME_LIBS"] == "0")
    po["__DEBUGRUNTIME_3"] = "";

  if(cfg["DEBUG_RUNTIME_LIBS"] == "1")
    po["__DEBUGRUNTIME_3"] = "d";

  //----------------------------------------------------

  if(cfg["BUILD"] == "debug")
    po["__OPTIMIZEFLAG_4"] = AddFlag("/Od");

  if(cfg["BUILD"] == "release")
    po["__OPTIMIZEFLAG_4"] = AddFlag("/O2");

  if(cfg["USE_THREADS"] == "0")
    po["__THREADSFLAG_7"] = "L";

  if(cfg["USE_THREADS"] == "1")
    po["__THREADSFLAG_7"] = "T";

  if(cfg["RUNTIME_LIBS"] == "dynamic")
    po["__RUNTIME_LIBS_8"] = "D";

  if(cfg["RUNTIME_LIBS"] == "static")
    po["__RUNTIME_LIBS_8"] = po["__THREADSFLAG_7"];

  //----------------------------------------------------

  if(cfg["USE_RTTI"] == "0")
    po["__RTTIFLAG_9"] = AddFlag("");

  if(cfg["USE_RTTI"] == "1")
    po["__RTTIFLAG_9"] = AddFlag("/GR");

  if(cfg["USE_EXCEPTIONS"] == "0")
    po["__EXCEPTIONSFLAG_10"] = AddFlag("");

  if(cfg["USE_EXCEPTIONS"] == "1")
    po["__EXCEPTIONSFLAG_10"] = AddFlag("/EHsc");

  //----------------------------------------------------

  if(cfg["BUILD"] == "debug" && cfg["DEBUG_RUNTIME_LIBS"] == "0")
    po["__NO_VC_CRTDBG_p"] = AddDefine("__NO_VC_CRTDBG__");

  if(cfg["BUILD"] == "release")
    po["__NO_VC_CRTDBG_p"] = AddDefine("__NO_VC_CRTDBG__");

  if(cfg["BUILD"] == "debug" && cfg["DEBUG_RUNTIME_LIBS"] == "0")
    po["__NO_VC_CRTDBG_p_1"] = AddResDefine("__NO_VC_CRTDBG__");

  if(cfg["BUILD"] == "release")
    po["__NO_VC_CRTDBG_p_1"] = AddResDefine("__NO_VC_CRTDBG__");

  if(cfg["WXUNIV"] == "1")
    po["__WXUNIV_DEFINE_p"] = AddDefine("__WXUNIVERSAL__");

  if(cfg["WXUNIV"] == "1")
    po["__WXUNIV_DEFINE_p_1"] = AddResDefine("__WXUNIVERSAL__");

  if(cfg["BUILD"] == "debug")
    po["__DEBUG_DEFINE_p"] = AddDefine("__WXDEBUG__");

  //        if (cfg["DEBUG_FLAG"] == "1")
  //            po["__DEBUG_DEFINE_p"] = AddDefine("__WXDEBUG__");

  if(cfg["BUILD"] == "debug")
    po["__DEBUG_DEFINE_p_1"] = AddResDefine("__WXDEBUG__");

  //        if (cfg["DEBUG_FLAG"] == "1")
  //            po["__DEBUG_DEFINE_p_1"] = AddResDefine("__WXDEBUG__");

  if(cfg["USE_EXCEPTIONS"] == "0")
    po["__EXCEPTIONS_DEFINE_p"] = AddDefine("wxNO_EXCEPTIONS");

  if(cfg["USE_EXCEPTIONS"] == "0")
    po["__EXCEPTIONS_DEFINE_p_1"] = AddResDefine("wxNO_EXCEPTIONS");

  if(cfg["USE_RTTI"] == "0")
    po["__RTTI_DEFINE_p"] = AddDefine("wxNO_RTTI");

  if(cfg["USE_RTTI"] == "0")
    po["__RTTI_DEFINE_p_1"] = AddResDefine("wxNO_RTTI");

  if(cfg["USE_THREADS"] == "0")
    po["__THREAD_DEFINE_p"] = AddDefine("wxNO_THREADS");

  if(cfg["USE_THREADS"] == "0")
    po["__THREAD_DEFINE_p_1"] = AddResDefine("wxNO_THREADS");

  if(cfg["UNICODE"] == "1")
    po["__UNICODE_DEFINE_p"] = AddDefine("_UNICODE");

  if(cfg["UNICODE"] == "1")
    po["__UNICODE_DEFINE_p_1"] = AddResDefine("_UNICODE");

  if(cfg["MSLU"] == "1")
    po["__MSLU_DEFINE_p"] = AddDefine("wxUSE_UNICODE_MSLU=1");

  if(cfg["MSLU"] == "1")
    po["__MSLU_DEFINE_p_1"] = AddResDefine("wxUSE_UNICODE_MSLU=1");

  if(cfg["USE_GDIPLUS"] == "1")
    po["__GFXCTX_DEFINE_p"] = AddDefine("wxUSE_GRAPHICS_CONTEXT=1");

  if(cfg["USE_GDIPLUS"] == "1")
    po["__GFXCTX_DEFINE_p_1"] = AddResDefine("wxUSE_GRAPHICS_CONTEXT=1");

  if(cfg["SHARED"] == "1")
    po["__DLLFLAG_p"] = AddDefine("WXUSINGDLL");

  if(cfg["SHARED"] == "1")
    po["__DLLFLAG_p_1"] = AddResDefine("WXUSINGDLL");

  Compiler::ProcessMain(po, cl, cfg, sho);

  // ### Variables, Part 2: ###
  // po["LIBDIRNAME"] = po["prefix"] + "/lib/" + GetName() + po["DIR_SUFFIX_CPU"] + "_" + po["LIBTYPE_SUFFIX"] +
  // cfg["CFG"];
  po["LIBDIRNAME"] =
      po["prefix"] + "/lib/" + cfg["COMPILER"] + cfg["COMPILER_VERSION"] + "_" + po["LIBTYPE_SUFFIX"] + cfg["CFG"];

  po["SETUPHDIR"] = po["LIBDIRNAME"] + "/" + po["PORTNAME"] + po["WXUNIVNAME"];
  po["SETUPHDIR"] += po["WXUNICODEFLAG"] + po["WXDEBUGFLAG"];

  po["cflags"] = "/M" + po["__RUNTIME_LIBS_8"] + po["__DEBUGRUNTIME_3"] + " " + AddDefine("WIN32");
  po["cflags"] += EasyMode(po["__DEBUGINFO_0"]) + EasyMode(po["____DEBUGRUNTIME_2_p"]);
  po["cflags"] +=
      EasyMode(po["__OPTIMIZEFLAG_4"]) + po["__NO_VC_CRTDBG_p"] + AddDefine("__WXMSW__") + po["__WXUNIV_DEFINE_p"];
  po["cflags"] += po["__DEBUG_DEFINE_p"] + po["__EXCEPTIONS_DEFINE_p"] + po["__RTTI_DEFINE_p"];
  po["cflags"] += po["__THREAD_DEFINE_p"] + po["__UNICODE_DEFINE_p"] + po["__MSLU_DEFINE_p"] + po["__GFXCTX_DEFINE_p"];
  po["cflags"] += AddIncludeDir(po["SETUPHDIR"]) + AddIncludeDir(po["prefix"] + "/include") + EasyMode(AddFlag("/W4")) +
                  EasyMode(AddIncludeDir(".")) + po["__DLLFLAG_p"] + AddDefine("_WINDOWS");
  po["cflags"] += EasyMode(AddIncludeDir(po["prefix"] + "/samples")) + EasyMode(AddDefine("NOPCH")) +
                  po["__RTTIFLAG_9"] + po["__EXCEPTIONSFLAG_10"];
  po["cflags"] += cfg["CPPFLAGS"] + " " + cfg["CXXFLAGS"] + " ";

  po["libs"] = EasyMode(AddFlag("/NOLOGO"));
  po["libs"] += cfg["LDFLAGS"] + " ";
  po["libs"] += EasyMode(po["__DEBUGINFO_1"]) + " " + po["LINK_TARGET_CPU"] + " ";
  po["libs"] += AddLinkerDir(po["LIBDIRNAME"]);
  po["libs"] += EasyMode(AddFlag("/SUBSYSTEM:WINDOWS"));
  po["libs"] += GetAllLibs(po);

  po["rcflags"] =
      AddResDefine("WIN32") + po["____DEBUGRUNTIME_2_p_1"] + po["__NO_VC_CRTDBG_p_1"] + AddResDefine("__WXMSW__");
  po["rcflags"] += po["__WXUNIV_DEFINE_p_1"] + po["__DEBUG_DEFINE_p_1"] + po["__EXCEPTIONS_DEFINE_p_1"];
  po["rcflags"] += po["__RTTI_DEFINE_p_1"] + po["__THREAD_DEFINE_p_1"] + po["__UNICODE_DEFINE_p_1"];
  po["rcflags"] += po["__MSLU_DEFINE_p_1"] + po["__GFXCTX_DEFINE_p_1"] + AddResIncludeDir(po["SETUPHDIR"]);
  po["rcflags"] += AddResIncludeDir(po["prefix"] + "/include") + EasyMode(AddResIncludeDir("."));
  po["rcflags"] += po["__DLLFLAG_p_1"] + AddResDefine("_WINDOWS");
  po["rcflags"] += EasyMode(AddResIncludeDir(po["prefix"] + "/samples"));

  po["release"] = cfg["WXVER_MAJOR"] + "." + cfg["WXVER_MINOR"];
  po["version"] = cfg["WXVER_MAJOR"] + "." + cfg["WXVER_MINOR"] + "." + cfg["WXVER_RELEASE"];
  po["basename"] = "wx" + po["PORTNAME"] + po["WXUNIVNAME"] + po["WX_RELEASE_NODOT"];
  po["basename"] += po["WXUNICODEFLAG"] + po["WXDEBUGFLAG"] + cfg["WX_LIB_FLAVOUR"];
  po["cc"] = m_programs.cc;
  po["cxx"] = m_programs.cxx;
  po["ld"] = m_programs.ld;

  GetVariablesValues(po, cl, cfg);

  ProcessEnd(po, cl, cfg);
}

void CompilerDMC::Process(Options& po, const CmdLineOptions& cl)
{
  /// Searchs for '<prefix>\build\msw\config.*' first
  // std::string cfg_first = po["prefix"] + "/build/msw/config." + GetName();

  /// config.* options
  // BuildFileOptions cfg(cfg_first);
  BuildFileOptions cfg;
  SetupHOptions sho;

  Compiler::ProcessStart(po, cl, cfg, sho);

  // ### Variables: ###
  po["WX_RELEASE_NODOT"] = cfg["WXVER_MAJOR"] + cfg["WXVER_MINOR"];
  if(po["WX_RELEASE_NODOT"].empty())
    po["WX_RELEASE_NODOT"] = "26";

  // ### Conditionally set variables: ###
  if(cfg["USE_GUI"] == "0")
    po["PORTNAME"] = "base";

  if(cfg["USE_GUI"] == "1")
    po["PORTNAME"] = "msw";

  if(cfg["BUILD"] == "debug")
    po["WXDEBUGFLAG"] = "d";

  //        if (cfg["DEBUG_FLAG"] == "1")
  //            po["WXDEBUGFLAG"] = "d";

  if(cfg["UNICODE"] == "1")
    po["WXUNICODEFLAG"] = "u";

  if(cfg["WXUNIV"] == "1")
    po["WXUNIVNAME"] = "univ";

  if(cfg["SHARED"] == "1")
    po["WXDLLFLAG"] = "dll";

  if(cfg["SHARED"] == "0")
    po["LIBTYPE_SUFFIX"] = "lib";

  if(cfg["SHARED"] == "1")
    po["LIBTYPE_SUFFIX"] = "dll";

  if(cfg["MONOLITHIC"] == "0")
    po["EXTRALIBS_FOR_BASE"] = "";

  if(cfg["MONOLITHIC"] == "1")
    po["EXTRALIBS_FOR_BASE"] = "";

  //----------------------------------------------------

  if(cfg["BUILD"] == "debug" && cfg["DEBUG_INFO"] == "default")
    po["__DEBUGINFO_0"] = AddFlag("-g");

  if(cfg["BUILD"] == "release" && cfg["DEBUG_INFO"] == "default")
    po["__DEBUGINFO_0"] = AddFlag("");

  if(cfg["DEBUG_INFO"] == "0")
    po["__DEBUGINFO_0"] = AddFlag("");

  if(cfg["DEBUG_INFO"] == "1")
    po["__DEBUGINFO_0"] = AddFlag("-g");

  if(cfg["BUILD"] == "debug" && cfg["DEBUG_INFO"] == "default")
    po["__DEBUGINFO_1"] = AddFlag("/DEBUG /CODEVIEW");

  if(cfg["BUILD"] == "release" && cfg["DEBUG_INFO"] == "default")
    po["__DEBUGINFO_1"] = AddFlag("");

  if(cfg["DEBUG_INFO"] == "0")
    po["__DEBUGINFO_1"] = AddFlag("");

  if(cfg["DEBUG_INFO"] == "1")
    po["__DEBUGINFO_1"] = AddFlag("/DEBUG /CODEVIEW");
  /*
          if (cfg["BUILD"] == "debug" && cfg["DEBUG_RUNTIME_LIBS"] == "default")
              po["____DEBUGRUNTIME_2_p"] = AddDefine("_DEBUG");

          if (cfg["BUILD"] == "release" && cfg["DEBUG_RUNTIME_LIBS"] == "default")
              po["____DEBUGRUNTIME_2_p"] = AddDefine("");

          if (cfg["DEBUG_RUNTIME_LIBS"] == "0")
              po["____DEBUGRUNTIME_2_p"] = AddDefine("");

          if (cfg["DEBUG_RUNTIME_LIBS"] == "1")
              po["____DEBUGRUNTIME_2_p"] = AddDefine("_DEBUG");

          if (cfg["BUILD"] == "debug" && cfg["DEBUG_RUNTIME_LIBS"] == "default")
              po["____DEBUGRUNTIME_2_p_1"] = AddResDefine("_DEBUG");

          if (cfg["BUILD"] == "release" && cfg["DEBUG_RUNTIME_LIBS"] == "default")
              po["____DEBUGRUNTIME_2_p_1"] = AddResDefine("");

          if (cfg["DEBUG_RUNTIME_LIBS"] == "0")
              po["____DEBUGRUNTIME_2_p_1"] = AddResDefine("");

          if (cfg["DEBUG_RUNTIME_LIBS"] == "1")
              po["____DEBUGRUNTIME_2_p_1"] = AddResDefine("_DEBUG");

          if (cfg["BUILD"] == "debug" && cfg["DEBUG_RUNTIME_LIBS"] == "default")
              po["__DEBUGRUNTIME_3"] = "d";

          if (cfg["BUILD"] == "release" && cfg["DEBUG_RUNTIME_LIBS"] == "default")
              po["__DEBUGRUNTIME_3"] = "";

          if (cfg["DEBUG_RUNTIME_LIBS"] == "0")
              po["__DEBUGRUNTIME_3"] = "";

          if (cfg["DEBUG_RUNTIME_LIBS"] == "1")
              po["__DEBUGRUNTIME_3"] = "d";
  */
  //----------------------------------------------------

  if(cfg["BUILD"] == "debug")
    po["__OPTIMIZEFLAG_4"] = AddFlag("-o+none"); // 2

  if(cfg["BUILD"] == "release")
    po["__OPTIMIZEFLAG_4"] = AddFlag("-o"); // 2
  /*
          if (cfg["USE_THREADS"] == "0")
              po["__THREADSFLAG_7"] = "L";

          if (cfg["USE_THREADS"] == "1")
              po["__THREADSFLAG_7"] = "T";
  */
  if(cfg["RUNTIME_LIBS"] == "dynamic")
    po["__RUNTIME_LIBS_8"] = "-ND"; // 5 // TODO: AddFlag?

  if(cfg["RUNTIME_LIBS"] == "static")
    po["__RUNTIME_LIBS_8"] = ""; // 5

  //----------------------------------------------------

  if(cfg["USE_RTTI"] == "0")
    po["__RTTIFLAG_9"] = AddFlag(""); // 6

  if(cfg["USE_RTTI"] == "1")
    po["__RTTIFLAG_9"] = AddFlag("-Ar"); // 6

  if(cfg["USE_EXCEPTIONS"] == "0")
    po["__EXCEPTIONSFLAG_10"] = AddFlag(""); // 7

  if(cfg["USE_EXCEPTIONS"] == "1")
    po["__EXCEPTIONSFLAG_10"] = AddFlag("-Ae"); // 7

  //----------------------------------------------------
  /*
          if (cfg["BUILD"] == "debug" && cfg["DEBUG_RUNTIME_LIBS"] == "0")
              po["__NO_VC_CRTDBG_p"] = AddDefine("__NO_VC_CRTDBG__");

          if (cfg["BUILD"] == "release" && cfg["DEBUG_FLAG"] == "1")
              po["__NO_VC_CRTDBG_p"] = AddDefine("__NO_VC_CRTDBG__");

          if (cfg["BUILD"] == "debug" && cfg["DEBUG_RUNTIME_LIBS"] == "0")
              po["__NO_VC_CRTDBG_p_1"] = AddResDefine("__NO_VC_CRTDBG__");

          if (cfg["BUILD"] == "release" && cfg["DEBUG_FLAG"] == "1")
              po["__NO_VC_CRTDBG_p_1"] = AddResDefine("__NO_VC_CRTDBG__");
  */
  if(cfg["WXUNIV"] == "1")
    po["__WXUNIV_DEFINE_p"] = AddDefine("__WXUNIVERSAL__");
  /*
          if (cfg["WXUNIV"] == "1")
              po["__WXUNIV_DEFINE_p_1"] = AddResDefine("__WXUNIVERSAL__");
  */
  if(cfg["BUILD"] == "debug")
    po["__DEBUG_DEFINE_p"] = AddDefine("__WXDEBUG__");

  //        if (cfg["DEBUG_FLAG"] == "1")
  //            po["__DEBUG_DEFINE_p"] = AddDefine("__WXDEBUG__");
  /*
          if (cfg["BUILD"] == "debug" && cfg["DEBUG_FLAG"] == "default")
              po["__DEBUG_DEFINE_p_1"] = AddResDefine("__WXDEBUG__");

          if (cfg["DEBUG_FLAG"] == "1")
              po["__DEBUG_DEFINE_p_1"] = AddResDefine("__WXDEBUG__");
  */
  if(cfg["USE_EXCEPTIONS"] == "0")
    po["__EXCEPTIONS_DEFINE_p"] = AddDefine("wxNO_EXCEPTIONS");
  /*
          if (cfg["USE_EXCEPTIONS"] == "0")
              po["__EXCEPTIONS_DEFINE_p_1"] = AddResDefine("wxNO_EXCEPTIONS");
  */
  if(cfg["USE_RTTI"] == "0")
    po["__RTTI_DEFINE_p"] = AddDefine("wxNO_RTTI");
  /*
          if (cfg["USE_RTTI"] == "0")
              po["__RTTI_DEFINE_p_1"] = AddResDefine("wxNO_RTTI");
  */
  if(cfg["USE_THREADS"] == "0")
    po["__THREAD_DEFINE_p"] = AddDefine("wxNO_THREADS");
  /*
          if (cfg["USE_THREADS"] == "0")
              po["__THREAD_DEFINE_p_1"] = AddResDefine("wxNO_THREADS");
  */
  if(cfg["UNICODE"] == "1")
    po["__UNICODE_DEFINE_p"] = AddDefine("_UNICODE");
  /*
          if (cfg["UNICODE"] == "1")
              po["__UNICODE_DEFINE_p_1"] = AddResDefine("_UNICODE");
  */
  if(cfg["MSLU"] == "1")
    po["__MSLU_DEFINE_p"] = AddDefine("wxUSE_UNICODE_MSLU=1");
  /*
          if (cfg["MSLU"] == "1")
              po["__MSLU_DEFINE_p_1"] = AddResDefine("wxUSE_UNICODE_MSLU=1");
  */
  if(cfg["USE_GDIPLUS"] == "1")
    po["__GFXCTX_DEFINE_p"] = AddDefine("wxUSE_GRAPHICS_CONTEXT=1");
  /*
          if (cfg["USE_GDIPLUS"] == "1")
              po["__GFXCTX_DEFINE_p_1"] = AddResDefine("wxUSE_GRAPHICS_CONTEXT=1");
  */
  if(cfg["SHARED"] == "1")
    po["__DLLFLAG_p"] = AddDefine("WXUSINGDLL");
  /*
          if (cfg["SHARED"] == "1")
              po["__DLLFLAG_p_1"] = AddResDefine("WXUSINGDLL");
  */
  Compiler::ProcessMain(po, cl, cfg, sho);

  // ### Variables, Part 2: ###
  // po["LIBDIRNAME"] = po["prefix"] + "/lib/" + GetName() + "_" + po["LIBTYPE_SUFFIX"] + cfg["CFG"];
  po["LIBDIRNAME"] =
      po["prefix"] + "/lib/" + cfg["COMPILER"] + cfg["COMPILER_VERSION"] + "_" + po["LIBTYPE_SUFFIX"] + cfg["CFG"];

  po["SETUPHDIR"] = po["LIBDIRNAME"] + "/" + po["PORTNAME"] + po["WXUNIVNAME"];
  po["SETUPHDIR"] += po["WXUNICODEFLAG"] + po["WXDEBUGFLAG"];

  po["cflags"] = EasyMode(po["__DEBUGINFO_0"]) + EasyMode(po["__OPTIMIZEFLAG_4"]);
  po["cflags"] += po["__RUNTIME_LIBS_8"] + " " + AddDefine("_WIN32_WINNT=0x0400");
  po["cflags"] += AddDefine("__WXMSW__") + po["__WXUNIV_DEFINE_p"];
  po["cflags"] += po["__DEBUG_DEFINE_p"] + po["__EXCEPTIONS_DEFINE_p"] + po["__RTTI_DEFINE_p"];
  po["cflags"] += po["__THREAD_DEFINE_p"] + po["__UNICODE_DEFINE_p"] + po["__MSLU_DEFINE_p"];
  po["cflags"] += po["__GFXCTX_DEFINE_p"];
  po["cflags"] += AddIncludeDir(po["SETUPHDIR"]) + AddIncludeDir(po["prefix"] + "/include") + EasyMode(AddFlag("-w-")) +
                  EasyMode(AddIncludeDir(".")) + po["__DLLFLAG_p"] + EasyMode(AddFlag("-WA"));
  po["cflags"] += EasyMode(AddIncludeDir(po["prefix"] + "/samples")) + EasyMode(AddDefine("NOPCH")) +
                  po["__RTTIFLAG_9"] + po["__EXCEPTIONSFLAG_10"];
  po["cflags"] += cfg["CPPFLAGS"] + " " + cfg["CXXFLAGS"] + " ";

  po["libs"] = EasyMode(AddFlag("/NOLOGO")) + EasyMode(AddFlag("/SILENT"));
  po["libs"] += EasyMode(AddFlag("/NOI")) + EasyMode(AddFlag("/DELEXECUTABLE"));
  po["libs"] += EasyMode(AddFlag("/EXETYPE:NT"));
  po["libs"] += cfg["LDFLAGS"] + " ";
  po["libs"] += EasyMode(po["__DEBUGINFO_1"]);
  po["libs"] += AddLinkerDir(po["LIBDIRNAME"] + "/");
  po["libs"] += EasyMode(AddFlag("/su:windows:4.0"));
  po["libs"] += GetAllLibs(po);

  po["rcflags"] = AddResDefine("_WIN32_WINNT=0x0400") + AddResDefine("__WXMSW__");
  po["rcflags"] += po["__WXUNIV_DEFINE_p"] + po["__DEBUG_DEFINE_p"] + po["__EXCEPTIONS_DEFINE_p"];
  po["rcflags"] += po["__RTTI_DEFINE_p"] + po["__THREAD_DEFINE_p"] + po["__UNICODE_DEFINE_p"];
  po["rcflags"] += po["__MSLU_DEFINE_p"] + po["__GFXCTX_DEFINE_p"] + AddResIncludeDir(po["SETUPHDIR"]);
  po["rcflags"] += AddResIncludeDir(po["prefix"] + "/include") + EasyMode(AddResIncludeDir("."));
  po["rcflags"] += po["__DLLFLAG_p"];
  po["rcflags"] += EasyMode(AddResIncludeDir(po["prefix"] + "/samples"));
  po["rcflags"] += EasyMode(AddFlag("-32")) + EasyMode(AddFlag("-v-"));

  po["release"] = cfg["WXVER_MAJOR"] + "." + cfg["WXVER_MINOR"];
  po["version"] = cfg["WXVER_MAJOR"] + "." + cfg["WXVER_MINOR"] + "." + cfg["WXVER_RELEASE"];
  po["basename"] = "wx" + po["PORTNAME"] + po["WXUNIVNAME"] + po["WX_RELEASE_NODOT"];
  po["basename"] += po["WXUNICODEFLAG"] + po["WXDEBUGFLAG"] + cfg["WX_LIB_FLAVOUR"];
  po["cc"] = m_programs.cc;
  po["cxx"] = m_programs.cxx;
  po["ld"] = m_programs.ld;

  GetVariablesValues(po, cl, cfg);

  ProcessEnd(po, cl, cfg);
}

void CompilerWAT::Process(Options& po, const CmdLineOptions& cl)
{
  /// Searchs for '<prefix>\build\msw\config.*' first
  // std::string cfg_first = po["prefix"] + "/build/msw/config." + GetName();

  /// config.* options
  // BuildFileOptions cfg(cfg_first);
  BuildFileOptions cfg;
  SetupHOptions sho;

  Compiler::ProcessStart(po, cl, cfg, sho);

  // ### Variables: ###
  po["WX_RELEASE_NODOT"] = cfg["WXVER_MAJOR"] + cfg["WXVER_MINOR"];
  if(po["WX_RELEASE_NODOT"].empty())
    po["WX_RELEASE_NODOT"] = "26";

  // ### Conditionally set variables: ###
  if(cfg["USE_GUI"] == "0")
    po["PORTNAME"] = "base";

  if(cfg["USE_GUI"] == "1")
    po["PORTNAME"] = "msw";

  if(cfg["BUILD"] == "debug")
    po["WXDEBUGFLAG"] = "d";

  //        if (cfg["DEBUG_FLAG"] == "1")
  //            po["WXDEBUGFLAG"] = "d";

  if(cfg["UNICODE"] == "1")
    po["WXUNICODEFLAG"] = "u";

  if(cfg["WXUNIV"] == "1")
    po["WXUNIVNAME"] = "univ";

  if(cfg["SHARED"] == "1")
    po["WXDLLFLAG"] = "dll";

  if(cfg["SHARED"] == "0")
    po["LIBTYPE_SUFFIX"] = "lib";

  if(cfg["SHARED"] == "1")
    po["LIBTYPE_SUFFIX"] = "dll";

  if(cfg["MONOLITHIC"] == "0")
    po["EXTRALIBS_FOR_BASE"] = "";

  if(cfg["MONOLITHIC"] == "1")
    po["EXTRALIBS_FOR_BASE"] = "";

  //---------till here, the same

  if(cfg["BUILD"] == "debug" && cfg["DEBUG_INFO"] == "default")
    po["__DEBUGINFO_0"] = AddFlag("-d2");

  if(cfg["BUILD"] == "release" && cfg["DEBUG_INFO"] == "default")
    po["__DEBUGINFO_0"] = AddFlag("-d0");

  if(cfg["DEBUG_INFO"] == "0")
    po["__DEBUGINFO_0"] = AddFlag("-d0");

  if(cfg["DEBUG_INFO"] == "1")
    po["__DEBUGINFO_0"] = AddFlag("-d2");

  if(cfg["BUILD"] == "debug" && cfg["DEBUG_INFO"] == "default")
    po["__DEBUGINFO_1"] = AddFlag("debug all"); // TODO: ???

  if(cfg["BUILD"] == "release" && cfg["DEBUG_INFO"] == "default")
    po["__DEBUGINFO_1"] = AddFlag(""); // TODO: ???

  if(cfg["DEBUG_INFO"] == "0")
    po["__DEBUGINFO_1"] = AddFlag(""); // TODO: ???

  if(cfg["DEBUG_INFO"] == "1")
    po["__DEBUGINFO_1"] = AddFlag("debug all"); // TODO: ???
  //---------from here, the same
  if(cfg["BUILD"] == "debug")
    po["__OPTIMIZEFLAG_2"] = AddFlag("-od");

  if(cfg["BUILD"] == "release")
    po["__OPTIMIZEFLAG_2"] = AddFlag("-ot -ox");
  //------from here, almost the same
  if(cfg["USE_THREADS"] == "0")
    po["__THREADSFLAG_5"] = AddFlag("");

  if(cfg["USE_THREADS"] == "1")
    po["__THREADSFLAG_5"] = AddFlag("-bm");
  //---------from here, simmilar to VC
  if(cfg["RUNTIME_LIBS"] == "dynamic")
    po["__RUNTIME_LIBS_6"] = AddFlag("-br");

  if(cfg["RUNTIME_LIBS"] == "static")
    po["__RUNTIME_LIBS_6"] = AddFlag("");
  //---------simmilar:
  if(cfg["USE_RTTI"] == "0")
    po["__RTTIFLAG_7"] = AddFlag("");

  if(cfg["USE_RTTI"] == "1")
    po["__RTTIFLAG_7"] = AddFlag("-xr");

  if(cfg["USE_EXCEPTIONS"] == "0")
    po["__EXCEPTIONSFLAG_8"] = AddFlag("");

  if(cfg["USE_EXCEPTIONS"] == "1")
    po["__EXCEPTIONSFLAG_8"] = AddFlag("-xs");

  Compiler::ProcessMain(po, cl, cfg, sho);

  //---------the same, but without resDefines:
  if(cfg["WXUNIV"] == "1")
    po["__WXUNIV_DEFINE_p"] = AddDefine("__WXUNIVERSAL__");

  if(cfg["BUILD"] == "debug")
    po["__DEBUG_DEFINE_p"] = AddDefine("__WXDEBUG__");

  //        if (cfg["DEBUG_FLAG"] == "1")
  //            po["__DEBUG_DEFINE_p"] = AddDefine("__WXDEBUG__");

  if(cfg["USE_EXCEPTIONS"] == "0")
    po["__EXCEPTIONS_DEFINE_p"] = AddDefine("wxNO_EXCEPTIONS");

  if(cfg["USE_RTTI"] == "0")
    po["__RTTI_DEFINE_p"] = AddDefine("wxNO_RTTI");

  if(cfg["USE_THREADS"] == "0")
    po["__THREAD_DEFINE_p"] = AddDefine("wxNO_THREADS");

  if(cfg["UNICODE"] == "1")
    po["__UNICODE_DEFINE_p"] = AddDefine("_UNICODE");

  if(cfg["MSLU"] == "1")
    po["__MSLU_DEFINE_p"] = AddDefine("wxUSE_UNICODE_MSLU=1");

  if(cfg["USE_GDIPLUS"] == "1")
    po["__GFXCTX_DEFINE_p"] = AddDefine("wxUSE_GRAPHICS_CONTEXT=1");

  if(cfg["SHARED"] == "1")
    po["__DLLFLAG_p"] = AddDefine("WXUSINGDLL");

  // ### Variables, Part 2: ###
  // po["LIBDIRNAME"] = po["prefix"] + "/lib/" + GetName() + "_" + po["LIBTYPE_SUFFIX"] + cfg["CFG"];
  po["LIBDIRNAME"] =
      po["prefix"] + "/lib/" + cfg["COMPILER"] + cfg["COMPILER_VERSION"] + "_" + po["LIBTYPE_SUFFIX"] + cfg["CFG"];

  po["SETUPHDIR"] = po["LIBDIRNAME"] + "/" + po["PORTNAME"] + po["WXUNIVNAME"];
  po["SETUPHDIR"] += po["WXUNICODEFLAG"] + po["WXDEBUGFLAG"];

  po["cflags"] = EasyMode(po["__DEBUGINFO_0"]) + EasyMode(po["__OPTIMIZEFLAG_2"]) + po["__THREADSFLAG_5"];
  po["cflags"] += po["__RUNTIME_LIBS_6"] + AddDefine("__WXMSW__") + po["__WXUNIV_DEFINE_p"];
  po["cflags"] += po["__DEBUG_DEFINE_p"] + po["__EXCEPTIONS_DEFINE_p"] + po["__RTTI_DEFINE_p"];
  po["cflags"] += po["__THREAD_DEFINE_p"] + po["__UNICODE_DEFINE_p"] + po["__MSLU_DEFINE_p"] + po["__GFXCTX_DEFINE_p"];
  po["cflags"] += AddIncludeDir(po["SETUPHDIR"]) + AddIncludeDir(po["prefix"] + "/include");
  po["cflags"] +=
      AddFlag("-wx") + AddFlag("-wcd=549") + AddFlag("-wcd=656") + AddFlag("-wcd=657") + AddFlag("-wcd=667");
  po["cflags"] += EasyMode(AddIncludeDir(".")) + po["__DLLFLAG_p"];
  po["cflags"] += EasyMode(AddIncludeDir(po["prefix"] + "/samples")) + AddDefine("NOPCH") + po["__RTTIFLAG_7"] +
                  po["__EXCEPTIONSFLAG_8"];
  po["cflags"] += cfg["CPPFLAGS"] + " " + cfg["CXXFLAGS"] + " ";

  po["lbc"] = "option quiet\n";
  po["lbc"] += "name $^@\n";
  po["lbc"] += "option caseexact\n";
  po["libs"] = cfg["LDFLAGS"] + " ";
  po["libs"] += EasyMode(po["__DEBUGINFO_1"]);
  po["libs"] += AddLinkerDir(po["LIBDIRNAME"]);
  po["lbc"] += "    libpath " + po["LIBDIRNAME"] + " system nt_win ref '_WinMain@16'";
  po["libs"] += GetAllLibs(po);

  po["rcflags"] = AddFlag("-q") + AddFlag("-ad") + AddFlag("-bt=nt") + AddFlag("-r");
  po["rcflags"] += AddResDefine("__WXMSW__") + po["__WXUNIV_DEFINE_p"];
  po["rcflags"] += po["__DEBUG_DEFINE_p"] + po["__EXCEPTIONS_DEFINE_p"];
  po["rcflags"] += po["__RTTI_DEFINE_p"] + po["__THREAD_DEFINE_p"] + po["__UNICODE_DEFINE_p"];
  po["rcflags"] += po["__MSLU_DEFINE_p"] + po["__GFXCTX_DEFINE_p"] + AddResIncludeDir(po["SETUPHDIR"]);
  po["rcflags"] += AddResIncludeDir(po["prefix"] + "/include") + EasyMode(AddResIncludeDir("."));
  po["rcflags"] += po["__DLLFLAG_p_1"];
  po["rcflags"] += EasyMode(AddResIncludeDir(po["prefix"] + "/samples"));

  po["release"] = cfg["WXVER_MAJOR"] + "." + cfg["WXVER_MINOR"];
  po["version"] = cfg["WXVER_MAJOR"] + "." + cfg["WXVER_MINOR"] + "." + cfg["WXVER_RELEASE"];
  po["basename"] = "wx" + po["PORTNAME"] + po["WXUNIVNAME"] + po["WX_RELEASE_NODOT"];
  po["basename"] += po["WXUNICODEFLAG"] + po["WXDEBUGFLAG"] + cfg["WX_LIB_FLAVOUR"];
  po["cc"] = m_programs.cc;
  po["cxx"] = m_programs.cxx;
  po["ld"] = m_programs.ld;

  GetVariablesValues(po, cl, cfg);

  ProcessEnd(po, cl, cfg);
}
