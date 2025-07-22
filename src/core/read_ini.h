#ifndef READ_INI_H
#define READ_INI_H
#include <cstdlib>
#include <map>
#include <string>

#include <physfs.h>

#include <util/file-manager.h>

namespace OpenGTA {

class ScriptParser {
public:
    explicit ScriptParser(const std::string &file);
    virtual ~ScriptParser();
    void loadLevel(uint32_t level);

private:
    std::map<uint32_t, int64_t> levels;
    Util::PhysFSFile pf;
    int64_t sectionEndOffset(int64_t start);

protected:
    std::string section_info;
    std::string section_vars;
    virtual void acceptDefinition(char *);
    virtual void acceptCommand(char *);
};

} // namespace OpenGTA
#endif
