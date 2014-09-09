@ECHO OFF
ECHO.
ECHO Downloading libitcoin_client dependencies from NuGet
CALL nuget.exe install ..\vs2013\libbitcoin_client\packages.config
ECHO.
CALL buildbase.bat ..\vs2013\libbitcoin_client.sln 12
ECHO.
PAUSE