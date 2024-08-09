@echo off

set GXT2CONV_PATH="gxt2conv.exe"

for %%f in (*.gxt2) do (
    %GXT2CONV_PATH% "%%f"
)