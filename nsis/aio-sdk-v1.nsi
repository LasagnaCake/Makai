# -*- coding: utf-8 -*-
Caption `Makai All-in-One Setup`
Name `the software`
BrandingText `Makai Superb Software Installer Wizard v1.0`
ManifestDPIAware true
ManifestLongPathAware true

SetFont "Roboto" 9

BGGradient 8800FF 3300AA FFFFFF
LicenseBkColor /windows

Icon "../etc/branding/logo-v1b.ico"

SetCompress force

OutFile `../output/package/win/install.exe`

InstallDir "$PROGRAMFILES\makai"

CompletedText `Installation successful!`

Function .onInit
	System::Call 'kernel32::CreateMutex(p 0, i 0, t "Makai Superb Mutex") p .r1 ?e'
	Pop $R0

	StrCmp $R0 0 +3
		MessageBox MB_OK|MB_ICONEXCLAMATION "The installer is already running."
		Abort
FunctionEnd

InstType "Full SDK"						IT_FULL
InstType "ART DevKit"					IT_ART_SDK
InstType "Anima Runtime Environment"	IT_ART_RE

PageEx license
	LicenseData `../LICENSE`
	LicenseForceSelection radiobuttons
	Caption ""
PageExEnd

PageEx components
PageExEnd

PageEx directory
	DirVerify leave
PageExEnd

Page instfiles

Section "Makai Framework Development Kit"
	SectionInstType ${IT_FULL}
	SetOutPath $INSTDIR
	SetOverwrite ifnewer
	File /r /oname=lib "../output/lib/*"
	File /r /oname=include "../output/include/*"
SectionEnd

Section "Anima Runtime Development Kit"
	SectionInstType ${IT_FULL} ${IT_ART_SDK}
	SetOutPath $INSTDIR
	SetOverwrite ifnewer
	File /r /oname=bin "../output/bin/*.dll"
	File /r /oname=bin/concerto.exe "../output/bin/concerto.exe"
	File /r /oname=bin/brevec.exe "../output/bin/brevec.exe"
	File /r /oname=bin/minimac.exe "../output/bin/minimac.exe"
SectionEnd

Section "Anima Runtime Environment"
	SectionInstType ${IT_FULL} ${IT_ART_SDK} ${IT_ART_RE}
	SetOutPath $INSTDIR
	SetOverwrite ifnewer
	File /r /oname=bin/art.exe "../output/bin/art.exe"
SectionEnd

Section "Register in PATH"
	SectionInstType ${IT_FULL} ${IT_ART_SDK} ${IT_ART_RE}
	EnVar::AddValue "path" "$INSTDIR/bin"
SectionEnd

Section
	WriteUninstaller "$INSTDIR/uninstaller.exe"
SectionEnd

UninstPage uninstConfirm
UninstPage instfiles

Section "Uninstall"
	Delete "$INSTDIR/bin/*"
	Delete "$INSTDIR/lib/*"
	Delete "$INSTDIR/include/*"
	RMDir $INSTDIR
SectionEnd
