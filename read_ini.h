#ifndef READ_INI_H
#define READ_INI_H
#include <string>
#include <map>
#include <cstdlib>
#include <physfs.h>

namespace OpenGTA {

  class ScriptParser {
    public:
      explicit ScriptParser(const std::string &file);
      virtual ~ScriptParser();
      void loadLevel(PHYSFS_uint32 level);
    private:
      std::map<PHYSFS_uint32, PHYSFS_sint64> levels;
      PHYSFS_file* fd;
      PHYSFS_sint64 sectionEndOffset(PHYSFS_sint64 start);
    protected:
      std::string  section_info;
      std::string  section_vars;
      virtual void acceptDefinition(char*);
      virtual void acceptCommand(char*);
  };

}
#endif
