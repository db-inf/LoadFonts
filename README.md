# LoadFonts
Load windows fonts apart of Windows registry and Windows\Fonts folder

I run a last Windows setup in a VirtualBox virtual machine on my linux host. I want both host and guest to have access to the same font collection, with as little duplication of files as possible. That is why the fonts are on the host's disks only.

Windows prefers the font files to be in the Windows\Fonts folder, which is not where I want them.

Font management software like Bitstream Font Navigator lets you put the files where you want them, and 'installs' fonts by placing a reference to them under the registry key `HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Fonts`. But Windows tries to load these fonts before it can access my network shares or VirtualBox shared folders, with as result that they are not loaded at all.

That made me write this simple program, that uses the Windows function `AddFontResource(filename)` to make the fonts available to every windows application. In the `Start Menu\Programs\Startup` folder of my setup, I placed a link to a copy of **LoadFonts.exe** on the shared network drive where my fonts are. Until now, that network share has always been ready by the time that link is called. Would Windows wait for that to be the case? A decent operating system would. If some day it does not, it would be easy to replace the direct link to LoadFonts.exe by a simple command file that waits for the network share to become available.

LoadFonts is simple to use. The program takes any number of parameters, which can be font file names, directory names with font files, preceded with `-R` to ask for recursion through the complete directory tree, or `-L` followed by a text file in which font file names and directory names are listed line by line. The formal syntax is
```
LoadFonts.exe [ fontfile | [-R] fontdirectory | -L fonts.lst ]...
     Font file can be a .ttf, .ttc, .fon, .fnt, .fot, .otf or .pfb type.
     For a Type 1 .pfb (font bits), also the .pfm (font metrics) and eventual
     .mmm (multiple master metrics) are loaded. Loading multiple master
     instances however is not supported.
     -R : load subdirectories of font directory recursively.
     Font directory : load all font files in it with the above extensions.
     Font list with fontdirectories (no recursion) and fontfiles, line by line.
```
LoadFonts is tested in Windows XP 32 bit, Windows 7 32 bit and Windows 10 64 bit. As font file names tend not to contain non-ascii characters, I did not bother to use the Unicode versions of the library function calls. [segin's LoadFonts](https://github.com/segin/loadfont) shows you how to change that.

I have no need for a counterpart `UnloadFonts` to `LoadFonts`. Making one seems trivial however. You would just have to replace the calls to `AddFontResource(filename)` by `RemoveFontResource(filename)`, reinterpret the return value as boolean instead of number of fonts loaded, and change the wording of some text strings that serve as user interface.

For compilation I used the command line Microsoft C compiler I once pulled out of the Express edition of Microsoft Visual Studio 2005, enriched with a stone age version of the Microsoft Windows SDK. I do not remember how I made that combol I am carrying it around since then, and it still works. But there is no fancy code in LoadFonts.c, so getting it compiled in newer setups should not be too difficult.

Be happy.
