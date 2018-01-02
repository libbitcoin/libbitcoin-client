@ECHO OFF
ECHO Downloading libbitcoin vs2017 dependencies from NuGet
CALL nuget.exe install ..\vs2017\libbitcoin-client\packages.config
CALL nuget.exe install ..\vs2017\libbitcoin-client-examples\packages.config
CALL nuget.exe install ..\vs2017\libbitcoin-client-test\packages.config
ECHO.
ECHO Downloading libbitcoin vs2015 dependencies from NuGet
CALL nuget.exe install ..\vs2015\libbitcoin-client\packages.config
CALL nuget.exe install ..\vs2015\libbitcoin-client-examples\packages.config
CALL nuget.exe install ..\vs2015\libbitcoin-client-test\packages.config
ECHO.
ECHO Downloading libbitcoin vs2013 dependencies from NuGet
CALL nuget.exe install ..\vs2013\libbitcoin-client\packages.config
CALL nuget.exe install ..\vs2013\libbitcoin-client-examples\packages.config
CALL nuget.exe install ..\vs2013\libbitcoin-client-test\packages.config
