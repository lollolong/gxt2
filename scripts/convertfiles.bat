@echo off

set GXT2CONV_PATH="gxt2conv.exe"

for /R %%d in (*.gxt2) do (
    %GXT2CONV_PATH% "%%d"
)