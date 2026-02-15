                     sc68 for Microsoft Visual C

                   It is part of the sc68 project
                                   
               (C) COPYRIGHT 1998-2014 Benjamin Gerard


SYNOPSIS

  This directory contains a solution and the associate project
  definition to build essential part of sc68 (libsc68 and its
  dependencies) with Microsoft Visual C within Microsoft Visual Studio
  C++ Express 2010.

  These projects are used by other Visual C studio based sc68
  sub-projects such as sc68-fb2k (sc68 for foobar2000) and sc68-dshow
  (sc68 filter for DirectShow).


FILES

  README ................. This file.
  sc68.sln ............... The solution for Visual C++ Express 2010
  config_msvc.h .......... Include by projects config.h
  unice68
   + unice68.vcxproj ..... Project definition for unice68
   + config.h ............ Replacement for otherwise generated config.h
  file68
   + file68.vcxproj ...... Project definition for file68
   + config.h ............ Replacement for otherwise generated config.h
  sc68
   + sc68.vcxproj ........ Project definition for sc68
   + config.h ............ Replacement for otherwise generated config.h
   + file68_features.h ... Replacement for generated file68_features.h
  zlib
   + zlib-md.lib ......... Precompiled zlib for MT/Debug runtime lib
   + zlib-mt.lib ......... Precompiled zlib for MT/retail runtime lib


LINKS

  sc68 website  <http://sc68.atari.org/>
  sc68 project  <http://sourceforge.net/p/sc68/>
  Visual Studio <http://www.visualstudio.com/>
  zlib          <http://www.zlib.net/>
