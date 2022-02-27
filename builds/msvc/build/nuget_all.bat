@ECHO OFF
ECHO Downloading libbitcoin vs2022 dependencies from NuGet
CALL nuget.exe install ..\vs2022\libbitcoin-client\packages.config
CALL nuget.exe install ..\vs2022\libbitcoin-client-examples\packages.config
CALL nuget.exe install ..\vs2022\libbitcoin-client-test\packages.config
ECHO.
ECHO Downloading libbitcoin vs2019 dependencies from NuGet
CALL nuget.exe install ..\vs2019\libbitcoin-client\packages.config
CALL nuget.exe install ..\vs2019\libbitcoin-client-examples\packages.config
CALL nuget.exe install ..\vs2019\libbitcoin-client-test\packages.config
ECHO.
ECHO Downloading libbitcoin vs2017 dependencies from NuGet
CALL nuget.exe install ..\vs2017\libbitcoin-client\packages.config
CALL nuget.exe install ..\vs2017\libbitcoin-client-examples\packages.config
CALL nuget.exe install ..\vs2017\libbitcoin-client-test\packages.config
