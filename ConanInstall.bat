set workspace=%1

for %%b in (Debug Release) do (
    conan install %workspace% ^
        --update ^
        --build missing ^
        --conf:host tools.cmake.cmake_layout:build_folder_vars="['settings.compiler']" ^
        --settings:host build_type=%%b ^
        --settings:host compiler.runtime_type=%%b
)
