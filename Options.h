#ifndef OPTIONS_INCLUDE
#define OPTIONS_INCLUDE

#include <iostream>
#include <string>
#include <cstdlib>

#include <map>
#include <fstream>
#include <algorithm>
#include <vector>

#include <windows.h>

typedef std::vector<std::string> VecLibs;
typedef std::map<std::string, std::string> MapVars;
typedef std::map<std::string, bool> StringBoolMap;

extern const std::string g_tokError;
extern const std::string g_tokWarning;

// -------------------------------------------------------------------------------------------------

/// Program options
class Options
{
  public:
  std::string& operator[](const std::string& key)
  {
    return m_vars[key];
  }

  bool KeyExists(const std::string& key) const
  {
    return m_vars.count(key) != 0;
  }

  const std::string& KeyValue(const std::string& key) const
  {
    return m_vars.find(key)->second;
  }

  MapVars& GetVars()
  {
    return m_vars;
  }

  protected:
  virtual void Split(const std::string& line);

  MapVars m_vars;
};

// -------------------------------------------------------------------------------------------------

/// File build.cfg/config.* options
class BuildFileOptions : public Options
{
  public:
  BuildFileOptions()
  {
  }

  BuildFileOptions(const std::string& filepath)
  {
    Parse(filepath);
  }

  bool Parse(const std::string& filepath, bool bDeleteAllVars = false);

  protected:
  /*
      void split(const std::string& line)
      {
        size_t sep = line.find('=');
        if(sep != std::string::npos)
        {
          std::string key = line.substr(0, sep);
          std::string val = line.substr(sep + 1, line.size() - sep-1);

          // trim whitespaces
          val.erase(val.find_last_not_of(" \n\r\t\v") + 1);
          val.erase(0, val.find_first_not_of(" \n\r\v\t"));
          m_vars[key] = val;
        }
      }
  */
};

// -------------------------------------------------------------------------------------------------

/// File setup.h options
class SetupHOptions
{
  public:
  SetupHOptions()
  {
  }
  SetupHOptions(const std::string& filepath)
  {
    Parse(filepath);
  }

  bool Parse(const std::string& filepath);

  bool& operator[](const std::string& key)
  {
    return m_vars[key];
  }

  bool KeyExists(const std::string& key) const
  {
    return m_vars.count(key) != 0;
  }

  bool KeyValue(const std::string& key) const
  {
    return m_vars.find(key)->second;
  }

  StringBoolMap& GetVars()
  {
    return m_vars;
  }

  public:
  void PrintDebug()
  {
    std::cout << "DEBUG: setup.h contents BEGIN -------------------------------------------------" << std::endl;
    for(StringBoolMap::iterator it = m_vars.begin(); it != m_vars.end(); ++it)
      std::cout << it->first << "=" << it->second << std::endl;
    std::cout << "DEBUG: setup.h contents END ---------------------------------------------------" << std::endl;
  }

  protected:
  void Split(const std::string& line);

  StringBoolMap m_vars;
};

// -------------------------------------------------------------------------------------------------

/// Command line options
class CmdLineOptions : public Options
{
  protected:
  VecLibs m_libs;

  public:
  CmdLineOptions()
  {
  }
  CmdLineOptions(int argc, char* argv[])
  {
    Parse(argc, argv);
  }

  bool ValidArgs();

  bool Parse(int argc, char* argv[])
  {
    // argv[0] is program, it is not needed
    for(int i = 1; i < argc; ++i)
    {
      std::string line;
      line = argv[i];
      Split(line);

      if(line == "--libs")
      {
        std::string libsList;
        if((i + 1) < argc) // test if we have "--libs xx"
        {
          libsList = argv[i + 1];
        }
        else
        {
          libsList = "std"; // add
        }
        ParseLibs(libsList);
      }
    }
    return true;
  }

  VecLibs& GetLibs() // const
  {
    return m_libs;
  }

  protected:
  /*
  virtual void Split(const std::string& line)
  {
    size_t sep = line.find("=");
    if(sep != std::string::npos)
    {
      std::string key = line.substr(0, sep);
      std::string val = line.substr(sep + 1, line.size()- sep - 1);
      m_vars[key] = val;
    }
    else
      m_vars[line] = "";
  }
  */

  bool LibExists(const std::string& lib)
  {
    return std::find(m_libs.begin(), m_libs.end(), lib) != m_libs.end();
  }

  void RemoveLib(const std::string& lib)
  {
    std::vector<std::string>::iterator iter = std::find(m_libs.begin(), m_libs.end(), lib);
    if(iter != m_libs.end())
    {
      m_libs.erase(iter);
    }
  }

  void AddLib(const std::string& lib)
  {
    // adds the lib if its not present already
    if(!LibExists(lib))
      m_libs.push_back(lib);
  }

  void ParseLibs(const std::string libs);
};

#endif // OPTIONS_INCLUDE