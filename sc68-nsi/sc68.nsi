# -*- mode: conf; indent-tabs-mode: nil -*-
#
# @file   sc68.nsi
# @author https://sourceforge.net/users/benjihan
# @brief  Installer for sc68
#
# Time-stamp: <2015-09-02 11:15:00 ben>
# Init-stamp: <2013-07-29 04:43:45 ben>

#--------------------------------
!include "FileFunc.nsh"
!include "x64.nsh"
#--------------------------------

!Define HKLM_SASHIPA "SOFTWARE\sashipa"
!Define HKLM_SC68    "${HKLM_SASHIPA}\sc68"

Name          "${NAME}"
Caption       "${CAPTION} (${__DATE__})"
Icon          "${SRCDIR}/sc68-icon-32.ico"
UninstallIcon "${SRCDIR}/sc68-icon-32.ico"
OutFile       "${OUTFILE}"
CRCCheck      On
SetCompress   force
SetCompressor /SOLID lzma
InstProgressFlags smooth colored
SetOverwrite  On

# ----------------------------------------------------------------------
#
# Globals
#
# ----------------------------------------------------------------------

Var /GLOBAL vlcdir              # VLC base install dir
Var /GLOBAL Vlcfile             # Installed VLC plugin
Var /GLOBAL Vlcarc              # either 32 or 64
Var /GLOBAL wmpdir              # Winamp base install dir
Var /GLOBAL wmpfile             # Installed Winamp plugin
Var /GLOBAL foodir              # Foobar2000 base install dir
Var /GLOBAL foofile             # Installed Foobar2000 plugin
Var /GLOBAL winarch             # either 32 or 64

# ----------------------------------------------------------------------
#
# !!! MUST BE MACROS !!! (because uninstall needs them too)
#
# ----------------------------------------------------------------------

!macro SetArch
    IntOp $winarch 32 + 0
    ${If} ${RunningX64}
    IntOp $winarch 64 + 0
    ${EndIf}
!macroend

!macro SetDefaultInstallDir
    StrCpy $INSTDIR "$PROGRAMFILES32\sc68"
    IntCmp $winarch 32 +2
    StrCpy $INSTDIR "$PROGRAMFILES64\sc68"
!macroend

!macro FindWinamp
    StrCpy $wmpfile ""
    ClearErrors
    ReadRegStr $Wmpdir HKCU "Software\Winamp" ""
    IfErrors +3
    IfFileExists "$wmpdir\winamp.exe" 0 +2
    StrCpy $wmpfile "$wmpdir\Plugins\in_sc68.dll"
!macroend

!macro FindFoobar
    StrCpy $foofile ""
    ClearErrors
    ReadRegStr $foodir HKLM "SOFTWARE\foobar2000" "InstallDir"
    IfErrors +3
    IfFileExists "$foodir\foobar2000.exe" 0 +2
    StrCpy $foofile "$foodir\components\foo_sc68.dll"
!macroend

!macro FindVlc
    StrCpy $vlcfile ""
    ClearErrors
    ReadRegStr $vlcdir HKLM "Software\VideoLAN\VLC" "InstallDir"
    IfErrors +3
    IfFileExists "$Vlcdir\vlc.exe" 0 +2
    StrCpy $vlcfile "$Vlcdir\plugins\demux\libsc68_plugin.dll"
!macroend

!macro FindPlugins
    ${If} ${RunningX64}
    SetRegview 64
    !insertmacro FindVlc
    SetRegview lastused
    IntOp $vlcarc 64 + 0
    StrCmp "" $vlcfile 0 vlc_done
    ${Endif}
    IntOp $vlcarc 32 + 0
    !insertmacro FindVlc
vlc_done:

!ifdef DEBUG
    MessageBox MB_OK "vlc arc: [$vlcarc] dir: [$Vlcdir] dll: [$vlcfile]"
!endif
    !insertmacro FindWinamp
    !insertmacro FindFoobar
!macroend

!macro SetCommonVars
    !insertmacro SetArch
    !insertmacro SetDefaultInstallDir
    !insertmacro FindPlugins
!macroend

# ----------------------------------------------------------------------
#
# Functions
#
# ----------------------------------------------------------------------

# Inp: None
# Out: $INSTDIR
# Use: $0
Function FindPreviousInstall
    ClearErrors
    ReadRegStr $0 HKLM ${HKLM_SC68} "Install_Dir"
    IfErrors +2
    StrCpy $INSTDIR $0
FunctionEnd

# In:  $0 file-hnadle
# Out: $1 Word
# Use: $2
Function FileReadWord
    FileReadByte $0 $1           # $1 = byte[0]
    FileReadByte $0 $2           # $2 = byte[1]
    IntOp $2 $2 << 8             # $2 = byte[1]<<8
    IntOp $1 $1 + $2             # $1 = (byte[1]<<8) | byte[0]
FunctionEnd

# In:  $0 file-handle
# Out: $1 Dword
# Use: $2
Function FileReadDword
    Call FileReadWord           # $1 = word[0]
    push $1                     # push word[0]
    Call FileReadWord           # $1 = word[1]
    pop $2                      # $2 = word[0]
    IntOp $1 $1 << 16           # $1 = word[1]<<16
    IntOp $1 $1 + $2            # $1 = (word[1]<<16) | word[0]
FunctionEnd


!ifdef GUESS_PLATFORM_EXE

# Guess .exe architecture
# Inp: $0 exe path
# Out: $0 0:invalid 32:win32 64:win64
# Use: $2
Function GuessPlatformExe
    ClearErrors
    FileOpen $0 $0 r
    IfErrors notfound
    
    FileSeek $0 0x3C SET        # Seek to <Pointer to PE/Coff Header>
    Call FileReadDword          # $1 = <Pointer to PE/Coff Header>

    MessageBox MB_OK "Pointer to PE/Coff Header: is [$1]"

    IntOp $2 $1 + 0x18          # $2 = <Pointer to Standard Coff Fields>
    push $2                     # Push <Pointer to Standard Coff Fields>
    FileSeek $0 $1 SET          # Seek to <Pointer to PE/Coff Header>
    Call FileReadDword          # $1 = <PE signature>

    IntFmt $2 "0x%08X" $1
    MessageBox MB_OK "Signature $2"
    
    IntCmpU $1 0x00004550 0 notfound notfound
    pop $1                      # $1 = <Pointer to Standard Coff Fields>
    FileSeek $0 $1 SET          # Seek to <Pointer to Standard Coff Fields>
    Call FileReadWord           # $1 = <coff magic>

    IntFmt $2 "0x%04X" $1
    MessageBox MB_OK "magic $2"

    FileClose $0
    IfErrors NotFound
    
    IntOp $0 32 + 0
    IntCmpU $1 0x010B found
    IntOp $0 64 + 0
    IntCmpU $1 0x020B found
notfound:
    IntOp $0 0 + 0
found:
FunctionEnd   

!endif

# ----------------------------------------------------------------------
#
# Licence
#
# ----------------------------------------------------------------------

!ifndef DEBUG
LicenseBkColor /windows
LicenseText "license"
LicenseData ${SRCDIR}\..\COPYING
LicenseForceSelection checkbox
!endif

# ----------------------------------------------------------------------
#
# Pages
#
# ----------------------------------------------------------------------

!ifndef DEBUG
Page license
!endif
Page components
Page directory
Page instfiles

# ----------------------------------------------------------------------
#
# Plugins Sections
#
# ----------------------------------------------------------------------

SectionGroup /e "!sc68 plugins"

# WINAMP
Section "sc68 for winamp" s_wmp
    file /oname=$wmpfile ${WMPDLL}
SectionEnd

Section "sc68 for vlc (32-bit)" s_vlc32
    file /oname=$vlcfile ${VLC32DLL}
SectionEnd

Section "sc68 for vlc (64-bit)" s_vlc64
    file /oname=$vlcfile ${VLC64DLL}
SectionEnd

Section "sc68 for foobar2000" s_foo
    file /oname=$foofile ${FOODLL}
SectionEnd

SectionGroupEnd

# ----------------------------------------------------------------------
#
# Hidden Section to select outpath
#
# ----------------------------------------------------------------------

Section "-outpath"
    SetOutPath $INSTDIR
SectionEnd

# ----------------------------------------------------------------------
#
# CLI Sections
#
# ----------------------------------------------------------------------

SectionGroup "sc68 cli programs (win32)" s0_32

# ----------------------------------------------------------------------

Section "sc68 cli player" s1_32
    File "${WIN32DIR}\bin\sc68.exe"
SectionEnd

Section "ICE packer and depacker" s2_32
    File "${WIN32DIR}\bin\unice68.exe"
SectionEnd

Section "Info on sc68 files" s3_32
    File "${WIN32DIR}\bin\info68.exe"
SectionEnd

Section "Make sc68 file" s4_32
    File "${WIN32DIR}\bin\mksc68.exe"
SectionEnd

SectionGroupEnd

# ----------------------------------------------------------------------

SectionGroup "sc68 cli programs (win64)" s0_64

Section "sc68 cli player (w64)"  s1_64
    File "${WIN64DIR}\bin\sc68.exe"
SectionEnd

Section "ICE packer and depacker (w64)" s2_64
    File "${WIN64DIR}\bin\unice68.exe"
SectionEnd

Section "Info on sc68 files (w64)" s3_64
    File "${WIN64DIR}\bin\info68.exe"
SectionEnd

Section "Make sc68 file (w64)" s4_64
    File "${WIN64DIR}\bin\mksc68.exe"
SectionEnd

SectionGroupEnd

# ----------------------------------------------------------------------

SectionGroup "sc68 config and data"

Section "sc68 music replay"
    File /r "${DATADIR}\Replay"
SectionEnd

Section "sc68 config file"
    File "${DATADIR}\sc68.cfg"
SectionEnd

SectionGroupEnd


# ----------------------------------------------------------------------
#
# Registry Section
#
# ----------------------------------------------------------------------

Section "-registry"
    SectionIn  1 2 3 RO

    ## Write config keys for this programs
    WriteRegStr HKLM ${HKLM_SC68} "Install_Dir" "$INSTDIR"

    ## Write the uninstall keys for Windows
    
    WriteRegStr HKLM \
        "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\sc68" \
        "DisplayName" \
        "sc68"

    WriteRegStr HKLM \
        "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\sc68" \
        "UninstallString" \
        '"$INSTDIR\${UNINSTALL}"'

    WriteRegStr HKLM \
        "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\sc68" \
        "DisplayIcon" \
        '"$INSTDIR\${UNINSTALL}"'
        
    WriteRegDWORD HKLM \
        "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\sc68" \
        "NoModify" 1
        
    WriteRegDWORD HKLM \
        "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\sc68" \
        "NoRepair" 1
    
    WriteUninstaller "${UNINSTALL}"
  
SectionEnd

# ----------------------------------------------------------------------
#
# Uninstall Section
#
# ----------------------------------------------------------------------

Section "Uninstall"

    ## Remove registry config keys

    DeleteRegKey HKLM ${HKLM_SC68}
    DeleteRegKey /ifempty HKLM ${HKLM_SASHIPA}

    ## Remove registry uninstaller keys
    
    DeleteRegKey HKLM \
        "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\sc68"

    ## Remove files

    StrCmp "" $vlcfile +2 0
    Delete $vlcfile

    StrCmp "" $wmpfile +2 0
    Delete $wmpfile

    StrCmp "" $foofile +2 0
    Delete $foofile

    StrCmp "" $INSTDIR +2 0
    RMDir /r $INSTDIR

SectionEnd

# ----------------------------------------------------------------------
#
# Callbacks
#
# ----------------------------------------------------------------------

Function DisableSection
  push $1
  push $2
  IntOp $2 ${SF_SELECTED} ~
  SectionGetFlags $0 $1
  IntOp $1 $1 & $2
  IntOp $1 $1 | ${SF_RO}
  SectionSetFlags $0 $1
  pop $1
FunctionEnd

Function .onInit

    !insertmacro SetCommonVars
    
    Call FindPreviousInstall

    # Check and disable individual plugin sections

    # --------------------------------
    # Winamp

    StrCpy $0 ${s_wmp}
    StrCmp "" $wmpfile 0 +2
    Call DisableSection

    # --------------------------------
    # Foobar
    
    StrCpy $0 ${s_foo}
    StrCmp "" $foofile 0 +2
    Call DisableSection

    # --------------------------------
    # Vlc
    
    StrCmp "" $vlcfile disable_both_vlc
!ifdef GUESS_PLATFORM_EXE
    StrCpy $0 "$vlcdir\vlc.exe"
    call GuessPlatformExe
!else
    IntOp $0 $vlcarc + 0
!endif    
    IntCmpU $0 32 disable_vlc64
    IntCmpU $0 64 disable_vlc32
disable_both_vlc:
    StrCpy $0 ${s_vlc32}
    Call DisableSection
disable_vlc64:
    StrCpy $0 ${s_vlc64}
    Call DisableSection
    goto disable_vlc_end
disable_vlc32:
    StrCpy $0 ${s_vlc32}
    Call DisableSection
disable_vlc_end:

    # --------------------------------
    # Disable unwanted platform

    Push ${s0_64}
    Push ${s0_32}
    StrCmp $win "win32" +2
    Exch
    Pop $0
    pop $0
    Call DisableSection
    IntOp $0 $0 + 1
    Call DisableSection
    IntOp $0 $0 + 1
    Call DisableSection
    IntOp $0 $0 + 1
    Call DisableSection
    IntOp $0 $0 + 1
    Call DisableSection

FunctionEnd

Function un.onInit
    !insertmacro SetCommonVars
FunctionEnd
