/************************************************************************
 * Copyright (c) 2005-2007 tok@openlinux.org.uk                          *
 *                                                                       *
 * This file contains code derived from information copyrighted by       *
 * DMA Design. It may not be used in a commercial product.               *
 *                                                                       *
 * See license.txt for details.                                          *
 *                                                                       *
 * This notice may not be removed or altered.                            *
 ************************************************************************/
#include <cstring>
#include "log.h"
#include "read_ini.h"

namespace OpenGTA {

  ScriptParser::ScriptParser(const std::string &file) {
    fd = PHYSFS_openRead(file.c_str());
    if (!fd)
      ERROR("could not open file {} for reading!", file);
    else {
      INFO("* Loading script {} ...", file);
/*
      unsigned char v;
      unsigned char m = 0;
      char buffer[10];
      char* b = reinterpret_cast<char*>(&buffer);
      while(!PHYSFS_eof(fd)) {
        PHYSFS_read(fd, static_cast<void*>(&v), 1, 1);
        if (m) {
          if (v != ']') {
            *b = v;
            b++;
          }
          else {
            m = 0;
            *b = 0x00;
            b = reinterpret_cast<char*>(&buffer);
            levels[static_cast<PHYSFS_uint32>(atoi(buffer))] = PHYSFS_tell(fd) + 1;
            std::cout << buffer << " " << PHYSFS_tell(fd) + 1 << std::endl;
          }
        }
        if (v == '[') {
          m = 1;
        }
      }
*/
      const size_t buflen = 1024;
      char buffer[buflen+1];
      char numbuf[10];
      PHYSFS_sint64 fd_off = 0;
      while(!PHYSFS_eof(fd)) {
        memset(buffer, 0, buflen+1);
        PHYSFS_sint64 fd_off_add = PHYSFS_readBytes(fd, static_cast<void*>(buffer), buflen);

        char * buf_ptr = buffer;
        while (buf_ptr) {
          char * found_str = strchr(buf_ptr, '[');
          char * found_str_end = strchr(buf_ptr, ']');
          if (found_str && found_str_end) {
            memset(numbuf, 0, 10);
            strncpy(numbuf, found_str + 1, found_str_end - found_str - 1);
            //std::cout << numbuf << ": " << fd_off + found_str_end - buf_ptr + 2 << std::endl;
            levels[static_cast<PHYSFS_uint32>(atoi(numbuf))] = fd_off + found_str_end - buf_ptr + 2;
          }
          if ((found_str) && (!found_str_end)) {
            PHYSFS_seek(fd, PHYSFS_tell(fd) - 10);
            break;
          }
          if (found_str)
            buf_ptr = found_str + 1;
          else
            break;
        }
        fd_off += fd_off_add;
      }
    }
    INFO("{} sections indexed", levels.size());
  }
  /*
   * * Loading script MISSION.INI ... 1 4
2 84961
1001 195409
1002 224532
1003 256339
1004 288768
102 323745
103 446555
1101 568117
1102 607842
1103 648562
1104 690150
202 732582
203 910588
1201 1107740
1202 1147497
1203 1188235
1204 1229847
18 sections indexed
*/

  ScriptParser::~ScriptParser() {
    if (fd != nullptr)
      PHYSFS_close(fd);
    levels.clear();
  }

  PHYSFS_sint64 ScriptParser::sectionEndOffset(PHYSFS_sint64 start) {
    PHYSFS_sint64 offset = PHYSFS_fileLength(fd);
    for (const auto & level : levels) {
      if (level.second > start && level.second < offset)
        offset = level.second;
    }
    return offset;
  }

  void ScriptParser::loadLevel(PHYSFS_uint32 level) {
    auto i = levels.find(level);
    if (i == levels.end()) {
      WARN("not a valid level: {}", level);
      return;
    }
    PHYSFS_sint64 end_of_section = sectionEndOffset(i->second);
    PHYSFS_seek(fd, i->second);

    const size_t buf_len = 255; // +1
    char buffer[buf_len+1];
    PHYSFS_uint16 read_bytes = 255;
    PHYSFS_uint16 offset = 0;
    size_t num_lines_read = 0;
    bool first_part_of_section = true;
    while(PHYSFS_tell(fd) < end_of_section) {
      memset(buffer+offset, 0, read_bytes+1);
      PHYSFS_readBytes(fd, buffer + offset, read_bytes);
      char* line_start = buffer;
      while (true) {
        char* line_end = strchr(line_start, '\r');
        if (line_start && line_end) { 
          if (*(line_end - 1) == ' ')
            line_end--;
          *line_end = 0;
          while (*line_start == '\n' || *line_start == '\r' || *line_start == ' ')
            line_start++;
          if (strlen(line_start) > 0) {
            //std::cout <<"["<< line_start << "]" << strlen(line_start)<<std::endl;
            if (num_lines_read == 0)
              //std::cout << "level: " << line_start << std::endl;
              section_info = line_start;
            else if (num_lines_read == 1)
              //std::cout << "vars: " << line_start << std::endl;
              section_vars = line_start;
            else {
              if (strncmp(line_start, "-1", 2) == 0)
                first_part_of_section = false;
              else {
              if (first_part_of_section)
                acceptDefinition(line_start);
              else
                acceptCommand(line_start);
              }
            }

            ++num_lines_read;
          }
          line_start = line_end + 1;
          if (*line_start == '\n')
            ++line_start;
        }
        else
          break;
      }
      //std::cout << uint32(line_start) - uint32(buffer) << std::endl; 
      const auto start_casted = reinterpret_cast<uintptr_t>(line_start);
      const auto buffer_casted = reinterpret_cast<uintptr_t>(buffer);
      const auto begin_rest = static_cast<PHYSFS_uint32>(start_casted - buffer_casted);
      offset = buf_len - begin_rest;
      memmove(buffer, &buffer[begin_rest], buf_len - begin_rest);
      read_bytes = buf_len - offset;
      if (PHYSFS_tell(fd) + read_bytes > end_of_section)
        read_bytes = end_of_section - PHYSFS_tell(fd);
      //std::cout << buffer << std::endl;
    }
  }

  void ScriptParser::acceptDefinition(char* str) {
    INFO("def: {}", str);
  }

  void ScriptParser::acceptCommand(char* str) {
    INFO("cmd: {}", str);
  }

}
