#include "Options.h"

// -------------------------------------------------------------------------------------------------

void Options::Split(const std::string& Line)
{
  std::string line = Line;
  std::string whitespaces(" \t\f\v\n\r");
  std::string delimiter(" ?=");

  // delete whitespaces at tail
  std::size_t found = line.find_last_not_of(whitespaces);
  if(found != std::string::npos)
    line.erase(found + 1);
  else
  {
    // line.clear(); // line is all whitespace
    return;
  }

  // delete whitespaces at head
  found = line.find_first_not_of(whitespaces);
  if(found != std::string::npos)
  {
    line.erase(0, found);
  }

  found = line.find('=');
  if(found != std::string::npos)
  {
    std::string key = line.substr(0, found);
    std::string val = line.substr(found + 1, line.size() - found - 1);

    found = key.find_last_not_of(delimiter); // remove " ?"
    if(found != std::string::npos)
      key.erase(found + 1);

    found = val.find_first_not_of(whitespaces);
    if(found != std::string::npos)
    {
      val.erase(0, found);
    }

    m_vars[key] = val;
  }
  else
  {
    m_vars[line] = "";
  }
}

// -------------------------------------------------------------------------------------------------

void SetupHOptions::Split(const std::string& line)
{
  // it's a comment line
  if(line.find_first_of('/') != std::string::npos)
    return; // skips the line

  std::string lineCopy = line;

  // strip spaces and tabs
  lineCopy.erase(std::remove(lineCopy.begin(), lineCopy.end(), ' '), lineCopy.end());
  lineCopy.erase(std::remove(lineCopy.begin(), lineCopy.end(), '\t'), lineCopy.end());

  std::string tokDefine("#define");
  size_t posDefine = lineCopy.find_first_of(tokDefine);

  // it's a #define line
  if(posDefine != std::string::npos)
  {
    std::string key;
    bool val;

    // resolves val, checking if last char is 0 or 1
    char lastChar = lineCopy.at(lineCopy.length() - 1); // TODO: I don't like this line :P
    if(lastChar == '0')
      val = false;
    else if(lastChar == '1')
      val = true;
    else
      return; // skips the line

    // resolves key
    size_t startPos = posDefine + tokDefine.length();
    key = lineCopy.substr(startPos, lineCopy.length() - startPos - 1);

    // finally saves the parsed data!
    m_vars[key] = val;
  }
}

bool SetupHOptions::Parse(const std::string& filepath)
{
  m_vars.clear();

  std::ifstream file(filepath.c_str());
  if(file.is_open())
  {
    std::string line;
    while(!file.eof())
    {
      std::getline(file, line);

      // does the splitting/parsing
      Split(line);
    }
    file.close();

    if(!m_vars.empty())
      return true;
  }
  else
    std::cout << g_tokError << "Unable to open file '" << filepath.c_str() << "'." << std::endl;

  return false;
}

// -------------------------------------------------------------------------------------------------

bool BuildFileOptions::Parse(const std::string& filepath, bool bDeleteAllVars /* = false */)
{
  if(bDeleteAllVars)
    m_vars.clear();

  std::ifstream file(filepath.c_str());
  if(file.is_open())
  {
    std::string line;
    while(!file.eof())
    {
      std::getline(file, line);

      // it's a comment line
      if(line.find_first_of('#') != std::string::npos)
        continue;

      // strip spaces
      // line.erase( std::remove(line.begin(), line.end(), ' '), line.end() );

      Split(line);
    }
    file.close();

    if(!m_vars.empty())
      return true;
  }
  else
    std::cout << g_tokError << "Unable to open file '" << filepath.c_str() << "'." << std::endl;

  return false;
}

// -------------------------------------------------------------------------------------------------

void CmdLineOptions::ParseLibs(const std::string libs)
{
  std::string param = libs;

  // if the last parameter doesn't haves a -- switch
  if(param.find("--") == std::string::npos)
  {
    // saves in the vector, comma separated text like "text1,text2,text3,text4"
    while(true)
    {
      size_t comma = param.find(",");
      if(comma != std::string::npos)
      {
        m_libs.push_back(param.substr(0, comma));
        param = param.substr(comma + 1, param.size() - comma - 1);
      }
      else
      {
        if(param.empty() == false)
          m_libs.push_back(param);
        break;
      }
    }
  }

  // assuming magic keyword 'std' as a lib parameter for non-monolithic
  // magic keyword std: links with xrc,qa,html,adv,core,base_xml,base_net,base,aui,richtext
  if(m_libs.empty() || LibExists("std"))
  {
    // addLib("aui");
    AddLib("xrc");
    AddLib("html");
    AddLib("adv");
    AddLib("core");
    AddLib("xml");
    AddLib("net");
    AddLib("base");
    // addLib("rich");
  }

  // not really a lib...
  RemoveLib("std");
}

bool CmdLineOptions::ValidArgs()
{
  bool valid = KeyExists("--compiler") || KeyExists("--easymode") || KeyExists("--variable") ||
               KeyExists("--define-variable") || KeyExists("--prefix") || KeyExists("--wxcfg") || KeyExists("--libs") ||
               KeyExists("--cflags") || KeyExists("--cxxflags") || KeyExists("--cppflags") || KeyExists("--rcflags") ||
               KeyExists("--list") || KeyExists("--selected-config") || KeyExists("--debug") ||
               KeyExists("--unicode") || KeyExists("--static") || KeyExists("--universal") || KeyExists("--release") ||
               KeyExists("--version") || KeyExists("--basename") || KeyExists("--cc") || KeyExists("--cxx") ||
               KeyExists("--ld") || KeyExists("-v");

  // TODO: not all flags are being validated
  if(!valid)
  {
    if(m_vars.size() > 1 && !KeyExists("--help"))
      std::cout << g_tokError << "Unrecognised option: '" << m_vars.begin()->first << "'\n" << std::endl;

    std::cerr << "Usage: wx-config [options]\n";
    std::cerr << "Options:\n";
    std::cerr << "  --prefix[=DIR]              Path of the wxWidgets installation (ie. C:/wxWidgets2.6.3)\n";
    std::cerr << "  --wxcfg[=DIR]               Relative path of the build.cfg file (ie. gcc_dll/mswud)\n";
    //          std::cerr << "  --list                      Lists all the library configurations. [NOT IMPLEMENTED]\n";
    std::cerr << "  --cflags                    Outputs all pre-processor and compiler flags.\n";
    std::cerr << "  --cxxflags                  Same as --cflags but for C++.\n";
    std::cerr << "  --rcflags                   Outputs all resource compiler flags. [UNTESTED]\n";
    std::cerr << "  --libs                      Outputs all linker flags.\n";
    std::cerr << std::endl;
    std::cerr << "  --debug[=yes|no]            Uses a debug configuration if found.\n";
    std::cerr << "  --unicode[=yes|no]          Uses an unicode configuration if found.\n";
    std::cerr << "  --static[=yes|no]           Uses a static configuration if found.\n";
    std::cerr << "  --universal[=yes|no]        Uses an universal configuration if found.\n";
    //          std::cerr << "  --easymode[=yes|no]         Outputs warnings, and optimize flags.\n";
    std::cerr << "  --compiler[=gcc,dmc,vc]     Selects the compiler.\n";
    //          std::cerr << "  --variable=NAME             Returns the value of a defined variable.\n";
    //          std::cerr << "  --define-variable=NAME=VAL  Sets a global value for a variable.\n";
    std::cerr << "  --release                   Outputs the wxWidgets release number.\n";
    std::cerr << "  --version                   Outputs the wxWidgets version.\n";
    std::cerr << "  --basename                  Outputs the base name of the wxWidgets libraries.\n";
    std::cerr << "  --cc                        Outputs the name of the C compiler.\n";
    std::cerr << "  --cxx                       Outputs the name of the C++ compiler.\n";
    std::cerr << "  --ld                        Outputs the linker command.\n";
    std::cerr << "  --selected-config           Outputs the selected config (defined with WXWIN and WXCFG).\n";
    std::cerr << "  -v                          Outputs the revision of wx-config.\n";

    std::cerr << std::endl;
    std::cerr << "  Note that using --prefix is not needed if you have defined the \n";
    std::cerr << "  environmental variable WXWIN.\n";
    std::cerr << std::endl;
    std::cerr << "  Also note that using --wxcfg is not needed if you have defined the \n";
    std::cerr << "  environmental variable WXCFG.\n";
    std::cerr << std::endl;
  }

  return valid;
}