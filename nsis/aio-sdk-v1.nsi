# -*- coding: utf-8 -*-
Caption `Makai All-in-One Setup`
Name `Makai AiO SDK`
BrandingText center `Makai Superb Software Installer Wizard v1.0`
ManifestDPIAware true
ManifestLongPathAware true
AddBrandingImage left 200

SetCompress true

File /r /oname=bin `../output/bin/*`
File /r /oname=lib `../output/lib/*`
File /r /oname=include `../output/include/*`

OutFile `../output/package/win/install.exe`

CompletedText `Installation successful!`

PageEx license
	LicenseData `../LICENSE`
	LicenseForceSelection radiobuttons
PageExEnd

PageEx components
PageExEnd

PageEx directory
	DirVerify leave
PageExEnd

PageEx instfiles
PageExEnd

UninstPage uninstConfirm
UninstPage instfiles
