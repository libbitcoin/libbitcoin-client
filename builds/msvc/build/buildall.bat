@ECHO OFF
ECHO.
ECHO Downloading libitcoin-client dependencies from NuGet
CALL nuget.exe install ..\vs2013\libbitcoin-client\packages.config
ECHO.
CALL buildbase.bat ..\vs2013\libbitcoin-client.sln 12
ECHO.
PAUSE