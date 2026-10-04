VCCoop - Hamdan Edition (experimental)
======================================
1) Back up your working GTA Vice City 1.0 folder, especially dinput8.dll and VCCoop/interface.
2) Do NOT install a DLL until the Windows GitHub Actions compilation succeeds.
3) Extract the GitHub Actions artifact VCCoop-Hamdan-Install-Win32.zip.
4) Copy dinput8.dll next to gta-vc.exe on BOTH PCs (host and guest).
5) Copy VCCoop/interface into the game folder on BOTH PCs.
6) Add the desired options from HAMDAN-vccoop-options.ini under [VCCoop] in both existing vccoop.ini files; do NOT replace the entire INI or lose your nicknames and host address.
7) For the client weapons setting, use GarderArmes=0 on the client; it removes money too.
8) The VCCoop.exe launcher may update the mod and overwrite the custom DLL; keep a DLL backup and verify it after updates. Launch gta-vc.exe directly if the menu supports it.
9) Run a joint test with host and guest: npc targeting / guest death / map contacts / run animation. Test 30 FPS separately if vehicle entry fails at 60.
IMPORTANT: The test runner only validates small policy units. A successful Windows compile does NOT prove the in-game behavior of any feature. Mission failure works only in scripts that check the host's death and may not cover every mission. Run-speed adjustment changes animation rate and does not guarantee real world movement speed is reduced. This is NOT an official VCCoop release.
