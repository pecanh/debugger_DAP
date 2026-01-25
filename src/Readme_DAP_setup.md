# MACOS

On the Mac Intel computers you can use the following DAP adapters:

    * LLVM 14.0.6 lldb-vscode  /usr/local/opt/llvm/bin/lldb-vscode
    * https://github.com/vadimcn/vscode-lldb using the "./extension/adapter/codelldb" that is in the VSIX file (contents are zipped)

NOTES:

1. To use the LLDB debugger you need to compiler with either -gdwarf-2 or -gdwarf-4 instead of -ggdb to ensure that the Dwarf2 or Dwarf4 debug format is used.
    Be aware that the clang++ -Og will still optimise out some lines, so if you want to ensure nothing is optimized out use -O0 (alpha O and numeric 0).

2. You will need to stop the Mac converting down dash "--" to an emdash "—" (longer dash). The following web page has info on how to do this:
<https://superuser.com/questions/555628/how-to-stop-mac-to-convert-typing-double-dash-to-emdash>

From the page for Mac 10.9 and above:
Go to System Preferences, then "Language & Region" then click the "Keyboard Preference ..." button and to go to "Text" tab.
It is no longer a substitution, however, but instead on the right-hand side of the window there is a tickbox "Use smart quotes and dashes". Un-ticking this stops it changing two hyphens into an en-dash.

One little gotcha: you must exit then restart your editor to have this change take effect.

3. The following is a shell script to start the codelldb.exe DAP adapter with debugging output enabled. You will need to adjust it for your Mac configuration:

  ```shell
  #!/bin/sh
  if [ "$(id -u)" == "0" ]; then
      echo "You are root. Please run again as a normal user!!!"
      exit 1
  fi
  #export PYTHONHOME=C:\Users\andrew\AppData\Local\Programs\Python\Python310
  export RUST_LOG=debug,codelldb=debug
  export RUST_LOG_STYLE=always
  export RUST_BACKTRACE=full
  ./codelldb --port 12345
  ```

4. If you get any of the following errors when debugging C::B then "brew install mesa" and try again:

    * +[MTLIOAccelDevice registerDevices]: Zero Metal services found
    * [api] No Metal renderer available.
    * [render]Unable to create basic Accelerated OpenGL renderer.


# WINDOWS

On Windows you can use the following DAP adapters:

    * MSYS2 MingW64  C:\msys64\mingw64\bin\lldb-vscode.exe
    * MSYS2 Clang64  C:\msys64\clang64\bin\lldb-vscode.exe
    * LLVM 14.0.6 C:\llvm\bin\lldb-vscode.exe
    * https://github.com/vadimcn/vscode-lldb using the ".\extension\adapter\codelldb.exe" that is in the VSIX file (contents are zipped)

On Windows the following table shows the installation package to install:

|         DAP Adapter                   | Install package/URL
|:------------------------------------- |:----------------------------------------- |
| C:\msys64\mingw64\bin\lldb-vscode.exe | pacman -S mingw-w64-clang-x86_64-lldb     |
| C:\msys64\clang64\bin\lldb-vscode.exe | pacman -S mingw-w64-x86_64-lldb           |
| C:\msys64\mingw32\bin\lldb-vscode.exe | pacman -S mingw-w64-i686-lldb             |
| C:\msys64\clang32\bin\lldb-vscode.exe | pacman -S mingw-w64-clang-i686-lldb       |
| C:\llvm\bin\lldb-vscode.exe           | https://github.com/llvm/llvm-project      |
| .\extension\adapter\codelldb.exe      | https://github.com/vadimcn/vscode-lldb    |


On Windows you configure the different DAP adapters as follows for the debugger directory configuration setting:

|         DAP Adapter                   | Config setting                                                                           |
|:------------------------------------- |:---------------------------------------------------------------------------------------- |
| C:\msys64\mingw64\bin\lldb-vscode.exe | Use native paths                                                                         |
| C:\msys64\clang64\bin\lldb-vscode.exe | Needs C::B compilergcc change to work !!!!  "Use Linux paths on Windows no drive letter" |
| C:\llvm\bin\lldb-vscode.exe           | Need to spend time to see what is causing the launch request to fail                     |
| .\extension\adapter\codelldb.exe      | Use relative path compared to the executable                                             |

**NOTES:**

1. To use the LLDB debugger you need to compiler with either -gdwarf-2 or -gdwarf-4 instead of -ggdb to ensure that the Dwarf2 or Dwarf4 debug format is used.
    Be aware that the clang++ -Og will still optimise out some lines, so if you want to ensure nothing is optimized out use -O0 (alpha O and numeric 0).

2. Do not mix MingW64 and Clang64 compiler and/or debugger!

3. The compilergcc.cpp for clangc++ builds need to change parameters to use the Unix '/' for the DAP debugger to work. This can be done in the
    CompilerGCC::DoRunQueue() function, but to make it configurable more files need to change.

    I have a preliminary change for this that changes the following files:

        src\plugins\compilergcc\compilergcc.cpp
        src\plugins\compilergcc\compilergcc.h
        src\plugins\compilergcc\compileroptionsdlg.cpp,
        src\plugins\compilergcc\resources\compiler_options.xrc

    This change is **not** included in the DAP debugger patches as the change is in a core plugin.

4. You need to set the PYTHONHOME variable in the Debugger setting dialog for the appropriate DAP adapter you use above as follows:
    * C:\msys64\mingw64\bin\lldb-vscode.exe       PYTHONHOME = C:\msys64\mingw64
    * C:\msys64\clang64\bin\lldb-vscode.exe       PYTHONHOME = C:\msys64\clang64
    * C:\llvm\bin\lldb-vscode.exe                 PYTHONHOME = C:\Users\<username>\AppData\Local\Programs\Python\Python310
    * .\extension\adapter\codelldb.exe            PYTHONHOME = C:\Users\<username>\AppData\Local\Programs\Python\Python310

5. The following is a batch file to start the codelldb.exe DAP adapter with debugging output enabled. You will need to adjust it for your Windows configuration:

```bat
echo on
@setlocal
@SET CurrentDir="%CD%"
@set PYTHONHOME=C:\Users\<username>\AppData\Local\Programs\Python\Python310\
@set RUST_LOG=debug,codelldb=debug
@set RUST_LOG_STYLE=always
@set RUST_BACKTRACE=full
@cd /d C:\vscode-lldb\extension\adapter
codelldb.exe --port 12345
cd /d %CurrentDir%
@endlocal
```

6. The following is a batch file to start the MinGW64 lldb-vscode.exe DAP adapter

```bat
echo on
set PYTHONHOME=C:\msys64\mingw64
C:\msys64\mingw64\bin\lldb-vscode.exe --port 12345
```

7. The following is a batch file to start the Clang64 lldb-vscode.exe DAP adapter

```bat
echo on
set PYTHONHOME=C:\msys64\clang64
C:\msys64\clang64\bin\lldb-vscode.exe --port 12345
```

8. The following is a batch file to start the LLVM lldb-vscode.exe DAP adapter

```bat
echo on
set PYTHONHOME=C:\Users\<username>\AppData\Local\Programs\Python\Python310\
C:\LLVM\bin\lldb-vscode.exe --port 12345
```

9. It is assumed that Windows 32 will work similar to x64, but has not been tested. zYou will need to adapt the instructions as required.

# LINUX
WIP:

Test lldb-vscode can run:

```sh
export PYTHONHOME=/usr/lib/python3.10
/usr/bin/lldb-vscode-14 --port 12345
export PATH="$PATH:/usr/bin/python3"
export PYTHONHOME=/usr/bin/python3
```

# Installing vscode-lldb

To install the vscode-lldb debugger you can us the following steps:

1. Download the applicable release for your OS on from the following Github Repo:
    <https://github.com/vadimcn/vscode-lldb>

2. Unzip the codelldb-*.vsix file

3. In the C::B DAP debugger configuration set the DAP executable to the following file that was unzipped :
    .\extension\adapter\codelldb[.exe]