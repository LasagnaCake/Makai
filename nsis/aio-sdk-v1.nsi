# -*- coding: utf-8 -*-
Caption `Makai All-in-One Setup`
Name `Makai AiO SDK`
BrandingText `Makai Superb Software Installer Wizard v1.0`
ManifestDPIAware true
ManifestLongPathAware true
AddBrandingImage left 200

InstType "Full SDK"		IT_FULL
InstType "ART DevKit"	IT_ART_SDK
InstType "ART Runtime"	IT_ART_RE

SetCompress force

OutFile `../output/package/win/install.exe`

CompletedText `Installation successful!`

PageEx license
	LicenseData `../LICENSE`
	LicenseForceSelection radiobuttons
PageExEnd

PageEx components
PageExEnd

Section "Full SDK"
	SectionInstType ${IT_FULL} RO
	SetOutPath $INSTDIR
	File /r /oname=lib "../output/lib/*"
	File /r /oname=include "../output/include/*"
SectionEnd

Section "Anima Runtime Development Kit"
	SectionInstType ${IT_FULL} ${IT_ART_SDK} RO
	SetOutPath $INSTDIR
	File /r /oname=bin "../output/bin/*.dll"
	File /r /oname=bin/concerto.exe "../output/bin/concerto.exe"
	File /r /oname=bin/brevec.exe "../output/bin/brevec.exe"
	File /r /oname=bin/minimac.exe "../output/bin/minimac.exe"
SectionEnd

Section "Anima Runtime Environment"
	SectionInstType ${IT_FULL} ${IT_ART_SDK} ${IT_ART_RE} RO
	SetOutPath $INSTDIR
	File /r /oname=bin/art.exe "../output/bin/art.exe"
SectionEnd

PageEx directory
	DirVerify leave
PageExEnd

Page instfiles

Section
	EnVar::AddValue "path" "$INSTDIR/bin"
	WriteUninstaller "$INSTDIR\uninstaller.exe"
SectionEnd

UninstPage uninstConfirm
UninstPage instfiles

Section "Uninstall"
Delete "$INSTDIR/bin/*"
Delete "$INSTDIR/lib/*"
Delete "$INSTDIR/include/*"
RMDir $INSTDIR
SectionEnd
