# LoadFonts
Load windows fonts apart of Windows registry and Windows\Fonts folder

I run a last Windows setup in a VirtualBox virtual machine on my linux host. I want both host and guest to have access to the same font collection, with as little duplication of files as possible. That is why the fonts are on the host's disks only.

Windows prefers the font files to be in the Windows\Fonts folder, which is not where I want them.

Font management software like Bitstream Font Navigator let you put the files where you want them, and places a reference to them under the registry key `HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Fonts`. But Windows tries to load these fonts before my network shares or VirtualBox shared folders are ready, with as result that they are not loaded at all.

That made me write this simple program, that uses the Windows function `AddFontResource(filename)` to make the fonts available to every windows application. My setup has a link to **LoadFonts.exe** in my `Start Menu\Programs\Startup` folder (or whatever it is called in English, I am not sure about that). Until now, my shared network drive that holds the fonts I want has always been ready by the time LoadFonts.exe is run from this link.

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

For compilation I used a command line Microsoft C compiler extracted from the Microsoft Visual Studio 2005 Express edition, enriched with some Microsoft Windows SDK. I know, stone age. I do not remember how I extracted it, I am just carrying it around since then. And it still works. But there is no fancy code in LoadFonts.c, so getting it compiled in newer setups should not be too difficult.

Have fun with it.
