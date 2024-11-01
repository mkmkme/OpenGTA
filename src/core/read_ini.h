#ifndef READ_INI_H
#define READ_INI_H
#include <cstdlib>
#include <map>
#include <string>

#include <physfs.h>

#include <core/numeric-types.h>

#include <util/file-manager.h>

namespace OpenGTA {

class ScriptParser {
public:
    explicit ScriptParser(const std::string &file);
    virtual ~ScriptParser();
    void loadLevel(UInt32 level);

private:
    std::map<UInt32, Int64> levels;
    Util::PhysFSFile pf;
    Int64 sectionEndOffset(Int64 start);

protected:
    std::string section_info;
    std::string section_vars;
    virtual void acceptDefinition(char *);
    virtual void acceptCommand(char *);
};

} // namespace OpenGTA
#endif
