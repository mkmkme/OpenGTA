$env:CMAKE_C_COMPILER_LAUNCHER = 'sccache'
$env:CMAKE_CXX_COMPILER_LAUNCHER = 'sccache'
$env:CMAKE_GENERATOR = 'Ninja'
$env:CMAKE_BUILD_PARALLEL_LEVEL = Get-CimInstance Win32_Processor | Select-Object -ExpandProperty NumberOfLogicalProcessors
$env:CMAKE_BUILD_TYPE = 'RelWithDebInfo'
$env:CMAKE_EXPORT_COMPILE_COMMANDS = 'ON'
