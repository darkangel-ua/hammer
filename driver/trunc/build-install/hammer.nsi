;NSIS Modern User Interface version 1.70
;Start Menu Folder Selection Example Script                                                  
;Written by Joost Verburg

;--------------------------------
;Include Modern UI

  !include "MUI.nsh"           
  !include "EnvVarUpdate.nsh"
;--------------------------------
;General

  ;Name and file
  Name $(product_name)
  OutFile "Hammer_setup.exe"
  Caption "Hammer Setup"
  ;Default installation folder
  InstallDir "$PROGRAMFILES\Hammer"
  
;--------------------------------
;Variables

  Var StartMenuFolder

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages

;  !insertmacro MUI_PAGE_LICENSE "${NSISDIR}\Contrib\Modern UI\License.txt"
;  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  
;  Start Menu Folder Page Configuration
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU" 
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\Hammer" 
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
;  !define MUI_STARTMENUPAGE_DEFAULTFOLDER "$(product_name)"

  !insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"
  LangString product_name ${LANG_ENGLISH} "Hammer"

;--------------------------------
;Installer Sections

Section "Dummy Section" SecDummy

  SetOutPath "$INSTDIR\bin"
  File template\win32\*.exe
  SetOutPath "$INSTDIR\scripts"
  File ..\scripts\startup.ham
  SetOutPath "$INSTDIR\eclipse-cdt-templates"
  File ..\eclipse-cdt-templates\.*
  SetOutPath "$INSTDIR\doc"
  File ..\doc\user_manual.pdf
  SetOutPath "$INSTDIR"

  ;Add hammer to PATH
  ${EnvVarUpdate} $0 "PATH" "A" "HKCU" "$INSTDIR\bin"

  ;Store installation folder
  WriteRegStr HKCU "Software\Hammer" "" $INSTDIR
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    
    ;Create shortcuts
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortCut  "$SMPROGRAMS\$StartMenuFolder\Hammer User Manual.lnk" "$INSTDIR\doc\user_manual.pdf"
    CreateShortCut  "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
  
  !insertmacro MUI_STARTMENU_WRITE_END

SectionEnd

;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_SecDummy ${LANG_ENGLISH} "A test section."

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDummy} $(DESC_SecDummy)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END
 
;--------------------------------
;Uninstaller Section

Section "Uninstall"
   RMDir /r "$INSTDIR"
   ;Remove hammer to PATH
   ${un.EnvVarUpdate} $0 "PATH" "R" "HKCU" "$INSTDIR\bin"

   !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
   Delete "$SMPROGRAMS\$StartMenuFolder\Hammer User Manual.lnk"
   Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"
   RMDir "$SMPROGRAMS\$StartMenuFolder"
   DeleteRegKey HKCU "Software\Hammer"
SectionEnd
