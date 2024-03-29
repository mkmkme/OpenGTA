#include <filesystem>
#include <string>
#include "lua_vm.h"
#include "dataholder.h"
#include "file_helper.h"
#include "log.h"

std::string map_filename;
const char* script_file;

void on_exit() {
  SDL_Quit();
  PHYSFS_deinit();
}

void parse_args(int argc, char* argv[]) {
  if (argc != 3) {
    ERROR("invalid args");
    exit(1);
  }
  map_filename = std::string(argv[1]);
  script_file = argv[2];
}

void run_init(const char* prg_name) {
  PHYSFS_init(prg_name);

  const auto &data_path = Util::FileHelper::BaseDataPath();
  if (std::filesystem::exists(data_path))
    PHYSFS_mount(data_path.c_str(), nullptr, 1);
  else
    WARN("Could not load data-source: {}", data_path);

  PHYSFS_mount(PHYSFS_getBaseDir(), nullptr, 1);

}

void run_main() {
  OpenGTA::Script::LuaVM vm;

  OpenGTA::MainMsgLookup::Instance().load("ENGLISH.FXT");
  OpenGTA::ActiveMap::Instance().load(map_filename);
  OpenGTA::Map & loadedMap = OpenGTA::ActiveMap::Instance().get();
  vm.setMap(loadedMap);

  vm.runFile(script_file);
}

int main(int argc, char* argv[]) {
  if (argc > 1)
    parse_args(argc, argv);

  atexit(on_exit);

  run_init(argv[0]);
  try {
    run_main();
  } catch (const std::exception &e) {
    ERROR("Exception occured: {}", e.what());
    throw;
  }

  return 0;
}
