; pass parameter /DOUTDIR=path\to\installer	- the path where installer is written to
; pass parameter /DVERSION=x.x.x
; pass parameter /DAPPDIR=path\to\app\dir	- the path to exe and supporting files/directories
; pass parameter /DVCREDIST=path\to\vcredist - path to vcredist_x86.exe

!include "Library.nsh"

Name "FMS"
OutFile "${OUTDIR}\FMS-${VERSION}-win32-installer.exe"

InstallDir "$PROGRAMFILES\FMS"

Page directory
Page components
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

Var INSTSERVICE
Var STARTUP

InstType "Typical"

Section "Program Files"
	SectionIn 1 RO
	SetOutPath $INSTDIR
	
	Call ShutdownService
	Call ShutdownApplication
	
	File /r "${APPDIR}\*.*"
	
	;install VC 2005 redistributable and then delete
	File "${VCREDIST}\vcredist_x86.exe"
	nsExec::Exec '"$INSTDIR\vcredist_x86.exe" /Q'
	Delete "$INSTDIR\vcredist_x86.exe"
	
	WriteUninstaller "uninstall.exe"
SectionEnd

Section "Startup After Installation"
	SectionIn 1
	StrCpy $STARTUP "Y"
SectionEnd

Section /o "Install Service"
	nsExec::Exec '"$INSTDIR\fms.exe" /registerService'
	nsExec::Exec '"$INSTDIR\fms.exe" /servicestart=automatic'
	StrCpy $INSTSERVICE "Y"
SectionEnd

Section "Create Shortcuts"
	SectionIn 1
	StrCmp $INSTSERVICE "Y" CreateWebShortcut 0
		CreateShortCut "$DESKTOP\Start FMS.lnk" "$INSTDIR\fms.exe" "" "$INSTDIR\fms.exe"
	CreateWebShortcut:
		CreateShortCut "$DESKTOP\FMS Web Interface.lnk" "http://localhost:8080/" "" "$INSTDIR\fms.exe"
SectionEnd


; Service must be uninstalled before program files
Section "un.Uninstall Service"
	Call un.ShutdownService
	nsExec::Exec '"$INSTDIR\fms.exe" /unregisterService'
SectionEnd

Section "un.Uninstall Shortcuts"
	Delete "$DESKTOP\Start FMS.lnk"
	Delete "$DESKTOP\FMS Web Interface.lnk"
SectionEnd

Section "un.Uninstall Program Files"
	Call un.ShutdownApplication

	Delete "$INSTDIR\*.*"
	RMDir /r "$INSTDIR"
SectionEnd






Function .onInstSuccess
	StrCmp $STARTUP "Y" 0 EndInstSuccess
		StrCmp $INSTSERVICE "Y" 0 StartupApp
			services::SendServiceCommand 'start' 'fms'
			Goto InstSuccessOpen
		StartupApp:
			Exec "$INSTDIR\fms.exe"
		InstSuccessOpen:
		Sleep 3000
		ExecShell "open" "http://localhost:8080/"
	EndInstSuccess:
FunctionEnd

Function ShutdownService
	services::IsServiceRunning 'fms'
	Pop $0
	StrCmp $0 "Yes" 0 EndShutdownService
		services::SendServiceCommand 'stop' 'fms'
		Sleep 10000
	EndShutdownService:
FunctionEnd

Function ShutdownApplication
	Processes::FindProcess "fms.exe"
	StrCmp $R0 "1" 0 EndShutdownApplication
		Processes::KillProcess "fms.exe"
		Sleep 10000
	EndShutdownApplication:
FunctionEnd

Function un.ShutdownService
	services::IsServiceRunning 'fms'
	Pop $0
	StrCmp $0 "Yes" 0 unEndShutdownService
		services::SendServiceCommand 'stop' 'fms'
		Sleep 10000
	unEndShutdownService:
FunctionEnd

Function un.ShutdownApplication
	Processes::FindProcess "fms.exe"
	StrCmp $R0 "1" 0 unEndShutdownApplication
		Processes::KillProcess "fms.exe"
		Sleep 10000
	unEndShutdownApplication:
FunctionEnd
