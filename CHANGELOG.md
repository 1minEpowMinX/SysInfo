# Changelog

All notable changes to SysInfo and SysInfoExtension.

Generated from the commit history by [git-cliff](https://git-cliff.org) over
`cliff.toml`. Rewrite it with `git cliff -o CHANGELOG.md` rather than editing
it by hand.

## Unreleased


### Features

- **logging**: Carry structured payloads alongside the log message ([`c2eeb6c`](https://github.com/1minEpowMinX/SysInfo/commit/c2eeb6cca1cd7ff2fb266475b20ca922d6b24a0f))
- **sysinfo**: Emit device inventory event for elastic telemetry ([`5cd4088`](https://github.com/1minEpowMinX/SysInfo/commit/5cd40887bf531fb3672f2b053f5ce1d1019ab389))
- **ui**: Rebuild the About window on widgets and the system palette ([`ee2f8d5`](https://github.com/1minEpowMinX/SysInfo/commit/ee2f8d5b0d62b6aae0f4c83b02134121bd63062e))
- **assets**: Redraw the extension icon as an svg master ([`86d50f8`](https://github.com/1minEpowMinX/SysInfo/commit/86d50f8d462a16955aa1ff80caf2b73f87bb2598))
- **assets**: Replace the raster icon set with the generator output ([`e83e18e`](https://github.com/1minEpowMinX/SysInfo/commit/e83e18e76e4d41f04dcebaa51cf99365741bb081))
- **ui**: Point the manifests and the popup at the new icon set ([`8f126a1`](https://github.com/1minEpowMinX/SysInfo/commit/8f126a1db773dbaaf040d065d2fc39b4643b15d5))
- **ext**: Bundle the monospace face with the popup ([`13c7422`](https://github.com/1minEpowMinX/SysInfo/commit/13c74227fa161e8f425e775b0c92b1516ca272ea))
- **ext**: Rebuild the status tab to read top-down ([`23a02ed`](https://github.com/1minEpowMinX/SysInfo/commit/23a02ed573ef1ca0ed12a9ef6f90f2b4b4ad0b49))
- **ext**: Put the agent state ahead of the version chips ([`6c21d7f`](https://github.com/1minEpowMinX/SysInfo/commit/6c21d7f7649eecf0e83c304d234433d099bc4370))
- **ext**: Show the history times in the interface language ([`7801d8c`](https://github.com/1minEpowMinX/SysInfo/commit/7801d8c5363be51d858d22d68e3b93e64d5e7f75))
- **ext**: Report the failures that leave a form without the block ([`4498aa4`](https://github.com/1minEpowMinX/SysInfo/commit/4498aa49e7b61676eeec248e159cc8d0a1796d5f))
- **ext**: Draw every icon in both browsers from one PNG set ([`0de1db3`](https://github.com/1minEpowMinX/SysInfo/commit/0de1db30acd7e23bf82ec5728ee949d50d57b7df))

### Fixes

- **ext**: Prevent history entries for tickets that were not created from the form ([`ee691ce`](https://github.com/1minEpowMinX/SysInfo/commit/ee691ce480f77827d0747b5b7370e2f841cc09b4))
- **ext**: Prevent history entries for tickets that were not created from the form ([`4978a48`](https://github.com/1minEpowMinX/SysInfo/commit/4978a48581315b2c907001947b95d72d21d0f0eb))
- **sysinfo**: Bound native API parsing against malformed input ([`c3b66e3`](https://github.com/1minEpowMinX/SysInfo/commit/c3b66e33e096ac0a74e3387d441b08033361e641))
- **security**: Validate the extension id echoed into Access-Control-Allow-Origin ([`a885a52`](https://github.com/1minEpowMinX/SysInfo/commit/a885a528b70504c240f91c41e04d1d6bd725131d))
- **ui**: Open the tray guide only from its own notification ([`7ef37e5`](https://github.com/1minEpowMinX/SysInfo/commit/7ef37e5828751e78d0ba31f2e8b58926f7c1320c))
- **api**: Stop translating the values of the /systeminfo payload ([`a5745aa`](https://github.com/1minEpowMinX/SysInfo/commit/a5745aa87cdd06b49000ac92ab435f27e022f719))
- **ext**: Spell out a missing IP or uptime in the ticket text ([`683e950`](https://github.com/1minEpowMinX/SysInfo/commit/683e950079806cb7e0807ba05223a237667395fc))
- **ui**: Drop the stray variation selector from the tray-guide text ([`64ae8c0`](https://github.com/1minEpowMinX/SysInfo/commit/64ae8c02f850d2f8a2fdc9c780e46002c148aec0))
- **ui**: Name the tray tooltip field after the boot time it carries ([`01c5f04`](https://github.com/1minEpowMinX/SysInfo/commit/01c5f04a8f4cbdaae8f3ad76d1c9c5b8273f330d))
- **ext**: Spell out every missing field of the ticket block ([`6cb66b6`](https://github.com/1minEpowMinX/SysInfo/commit/6cb66b63d034e1966e31154c28b5e25c574be792))
- **ext**: Stop the popup settings seed from aliasing the defaults ([`0941e39`](https://github.com/1minEpowMinX/SysInfo/commit/0941e394521bc4a0a953f89ba18e158179696ce9))
- **ext**: Give every data row the same font ([`381c0fa`](https://github.com/1minEpowMinX/SysInfo/commit/381c0fae48ee9412bec1b178bf5dc3e3e095e8c2))
- **ext**: Stop asking for a weight the UI font does not have ([`cd7025f`](https://github.com/1minEpowMinX/SysInfo/commit/cd7025f53b856b66cce1d557891fec708151a96e))
- **ext**: Declare which language the popup is showing ([`78f9fb3`](https://github.com/1minEpowMinX/SysInfo/commit/78f9fb3bb2505d12f8ca1f11c8727db6e058d9f4))
- **ext**: Draw the chip's remove button in the interface font ([`a32be17`](https://github.com/1minEpowMinX/SysInfo/commit/a32be176431d3c37521aa1c115edb1e722811ce7))
- **ext**: Enlarge the version chip labels ([`26c5a41`](https://github.com/1minEpowMinX/SysInfo/commit/26c5a417164ff84d03f67647aa8d217d12414913))
- **ext**: Stop the short tabs from ending in dead space ([`56461f5`](https://github.com/1minEpowMinX/SysInfo/commit/56461f57d4817961f7bc743a19e310a7f52d1a71))
- **ext**: Stop clipping the settings tab ([`fd4c342`](https://github.com/1minEpowMinX/SysInfo/commit/fd4c3423396ffe7519fa261c596ef9b44bd4a320))
- **ext**: Record history only for tickets the form created ([`9f7db8b`](https://github.com/1minEpowMinX/SysInfo/commit/9f7db8b737e0bca689e2dbffe5f720e445f11f8f))
- **ext**: Report a missed ticket heading and anchor its selector ([`fec54d6`](https://github.com/1minEpowMinX/SysInfo/commit/fec54d69f1c1bbfd54caea4dc7368f0ff27aebc5))
- **ext**: Watch for submissions only on a whitelisted request form ([`26bfc9b`](https://github.com/1minEpowMinX/SysInfo/commit/26bfc9b1a0864e6fe75411f57b421fb3f2166b79))
- **ext**: Let a whitelist the user emptied close the insertion ([`9b35fe2`](https://github.com/1minEpowMinX/SysInfo/commit/9b35fe2b5485684b3b1b1a231c03f889521968cc))
- **ext**: Honour the field toggles in the inserted block ([`a3f3a09`](https://github.com/1minEpowMinX/SysInfo/commit/a3f3a09999242883e4382d80865106b701e5ecb7))
- **ui**: Report the tray-guide dismissal whichever way the window closes ([`d6a9c38`](https://github.com/1minEpowMinX/SysInfo/commit/d6a9c382bf441a90205bf99f5698556b791a0b65))
- **ext**: Settle the caller when a rejection carries no message ([`bd16798`](https://github.com/1minEpowMinX/SysInfo/commit/bd167984ef9bf9d540b94c506805656dfbc8b758))
- **ext**: Count the first attempt in the retry log ([`659c1ea`](https://github.com/1minEpowMinX/SysInfo/commit/659c1ea937638c75d4d654f925d19fe0c9f8619a))
- **ext**: Ship every asset the popup loads and the configuration it was packaged from ([`a2a7159`](https://github.com/1minEpowMinX/SysInfo/commit/a2a71591be7deb23dc09f96e6d9171516df86bc4))
- **sysinfo**: Stop including a header that cannot coexist with this namespace ([`b0cfb4b`](https://github.com/1minEpowMinX/SysInfo/commit/b0cfb4bd260c414e7563c4e52d2663894f95356e))

### Performance

- **sysinfo**: Skip string formatting for addresses that are discarded ([`af1d42c`](https://github.com/1minEpowMinX/SysInfo/commit/af1d42c54714c145296909d2351a7ef7186e6caf))
- **logging**: Open the Windows event source once per process ([`ef9c424`](https://github.com/1minEpowMinX/SysInfo/commit/ef9c424200e2e6ae7c6e870e1f7ff1a1433ab839))
- **api**: Cache the session snapshot and emit compact JSON ([`801d2bc`](https://github.com/1minEpowMinX/SysInfo/commit/801d2bc21ad8d4321fc1ee3b35f8e0dbd9dc8aed))
- **sysinfo**: Collect the device inventory off the startup path ([`5ecb9b6`](https://github.com/1minEpowMinX/SysInfo/commit/5ecb9b6d405b914d4e954c19c273e288e3540173))

### Translations

- Add the startup failure message and unify uk terminology ([`ba05dbb`](https://github.com/1minEpowMinX/SysInfo/commit/ba05dbb339751df3f0e34101c72af1477a7fbc52))
- Move the system details block to the presenter context ([`c5f8753`](https://github.com/1minEpowMinX/SysInfo/commit/c5f8753d6cb353f0ce2c101a91c88c7a53deaf7a))
- Translate the redesigned About window into ru and uk ([`1cfd07d`](https://github.com/1minEpowMinX/SysInfo/commit/1cfd07d5dd73026ce82058adc9d0e4a44c24a53f))
- Mark the About window labels finished and refresh the catalogues ([`15fd47c`](https://github.com/1minEpowMinX/SysInfo/commit/15fd47ce7f0cfc50d2e31b59a4007f15d382025d))
- Carry the copyright year as a placeholder ([`e1d0fce`](https://github.com/1minEpowMinX/SysInfo/commit/e1d0fce1aabc2a75acecf9f5da766a96a3a185df))
- Leave the build stamp of the About window untranslated ([`dc3ff07`](https://github.com/1minEpowMinX/SysInfo/commit/dc3ff070373891fe29d06b587ced146159378a92))
- **ext**: Substitute the agent host into the status line instead of translating it ([`b5554bd`](https://github.com/1minEpowMinX/SysInfo/commit/b5554bd22106cfbe82cccaaddbb810774ab78934))

### Refactoring

- **sysinfo**: Extract bootTime and add machine-readable accessors ([`7edf78c`](https://github.com/1minEpowMinX/SysInfo/commit/7edf78c6fda3f4b92ef51a6e1dafc11ed70837c2))
- **runtime**: Extract the single-instance lock into a testable guard ([`bd420e2`](https://github.com/1minEpowMinX/SysInfo/commit/bd420e2e4e8e7935bcd4029ae4ab4e8ccebbe257))
- **ui**: Give the dialogs their own translation contexts ([`5e91368`](https://github.com/1minEpowMinX/SysInfo/commit/5e913681f284eae9eb82398819c5635b3c39b77c))
- **app**: Move the dialogs, the tray gate and the delay out of App ([`3458764`](https://github.com/1minEpowMinX/SysInfo/commit/34587640311e1532333c121d7ff2d5056acbeebf))
- **ui**: Move the About system details into the presenter ([`70a50da`](https://github.com/1minEpowMinX/SysInfo/commit/70a50da33a884c9593c34a10041cfcf181bd0303))
- **app**: Give WelcomeNotifier ports for both of its collaborators ([`7f264a6`](https://github.com/1minEpowMinX/SysInfo/commit/7f264a6351dad29114bf215368ad167d55b22b24))
- **ui**: Let TrayGuide report the dismissal instead of storing it ([`ba89c07`](https://github.com/1minEpowMinX/SysInfo/commit/ba89c074098f5aa720980db7f23caa35a23497a7))
- **settings**: Narrow what the store exposes to each of its consumers ([`31c1d22`](https://github.com/1minEpowMinX/SysInfo/commit/31c1d22f387a8946aa1fd67b74f8c71b48330c3e))
- **sysinfo**: Split the hardware collectors per platform ([`9d507e3`](https://github.com/1minEpowMinX/SysInfo/commit/9d507e37d7fc3f5808a8bb23335abb659d91ccc8))
- **security**: Extract the endpoint access policy from the server ([`fa29aa3`](https://github.com/1minEpowMinX/SysInfo/commit/fa29aa307e31cf5d467fa2256a264298a46086cb))
- **app**: Put every collaborator of App behind a port ([`60f7663`](https://github.com/1minEpowMinX/SysInfo/commit/60f7663bedf86215346a08d41fdf3d8bef263555))
- **sysinfo**: Give the session snapshot a single owner ([`99bdb9a`](https://github.com/1minEpowMinX/SysInfo/commit/99bdb9a3136ef7a5ac0fc611ddcc4c7c51b3bd41))
- **settings**: Hand App each settings port it needs, not the store ([`38b5c27`](https://github.com/1minEpowMinX/SysInfo/commit/38b5c27bc38ffa25ed79195147485ac970fd4448))
- **app**: Drop the settings-path port and the whitelist App never read ([`1d1c273`](https://github.com/1minEpowMinX/SysInfo/commit/1d1c273df353306ae5bbd0372253452d539a8bd7))
- **sysinfo**: Rename Info::uptime to lastBootTime ([`05b3fa5`](https://github.com/1minEpowMinX/SysInfo/commit/05b3fa555ef0871f1429f8dbc5971569ab521db6))
- **ui**: Give every figure of the About window a name ([`65458ab`](https://github.com/1minEpowMinX/SysInfo/commit/65458abd12eae015d41ff0df0faeae569e5cd289))
- **ui**: Stamp the copyright year of the About window at build time ([`22a9699`](https://github.com/1minEpowMinX/SysInfo/commit/22a9699b4f1a820e549e57f13632f3a2a8d41fcc))
- **ext**: Name the boot time field after what it carries ([`72fe747`](https://github.com/1minEpowMinX/SysInfo/commit/72fe7475e5afb46ffe3faf219eac3c4a38402173))
- **ext**: Move the icon set into its own assets subfolder ([`14fa1af`](https://github.com/1minEpowMinX/SysInfo/commit/14fa1af192880f520be299eb5c5f13430ded1204))
- **ext**: Name the interface font stack once ([`27ae44b`](https://github.com/1minEpowMinX/SysInfo/commit/27ae44b12593ad82fc6a6af4f9f0fce8e3f047a9))
- **ext**: Move the data tab's padding out of the markup ([`16cda35`](https://github.com/1minEpowMinX/SysInfo/commit/16cda3508d308c1150afa5e5a16f5e38b6c7e501))
- **ext**: Drop the bundled monospace face ([`c268b38`](https://github.com/1minEpowMinX/SysInfo/commit/c268b383914dfbc8e828d97f486640c89332be4a))
- **ext**: Write the block through innerText alone ([`90d5bc1`](https://github.com/1minEpowMinX/SysInfo/commit/90d5bc1b7ddc1716d0d929efb1bd82904e698a7d))
- **ext**: Take the agent origin and the whitelist seeds from the build configuration ([`1a6ab4e`](https://github.com/1minEpowMinX/SysInfo/commit/1a6ab4ebe43cb1a792f23cf4b8bf896e1f36fcde))

### Documentation

- Bump version to 3.0.0 and add Windows Event Log registration step ([`58c5275`](https://github.com/1minEpowMinX/SysInfo/commit/58c52750dfd1c8fd063249ee3ea61acd39c51f8a))
- Update bundled Qt version to 6.11.1 ([`6ee6ef9`](https://github.com/1minEpowMinX/SysInfo/commit/6ee6ef976a598f82592839c2ac2c90cfa8b785eb))
- Update LinkedIn profile URL; sync author name with code ([`5b2fb6a`](https://github.com/1minEpowMinX/SysInfo/commit/5b2fb6aa6b75e41da07699a47bab391ae6b76652))
- Fix windows log setup in step 2 ([`5577995`](https://github.com/1minEpowMinX/SysInfo/commit/5577995f09f26a84f06741461486479ed69ad3c4))
- Rewrite docstrings in third person and drop the narrative ([`8d9fcb6`](https://github.com/1minEpowMinX/SysInfo/commit/8d9fcb68beb384923876a2ec6249a6261d05e945))
- Document the remaining helpers and correct the stale docstrings ([`e6dacd3`](https://github.com/1minEpowMinX/SysInfo/commit/e6dacd3209be8b4305debfb8cda8db0519418407))
- **logging**: Record why Logger is a static facade and what it costs ([`39a6a4e`](https://github.com/1minEpowMinX/SysInfo/commit/39a6a4e7fc184de793d936ee486a618b8cf60373))
- Move design rationale out of docstrings into the implementations ([`b0d2f58`](https://github.com/1minEpowMinX/SysInfo/commit/b0d2f581fd16f01131c52312c90da23f2c19143b))
- **build**: Restore the two ordering traps in CMakeLists ([`5650659`](https://github.com/1minEpowMinX/SysInfo/commit/5650659e9a1eaf07ccb3c717b395780aa80e8fd8))
- **api**: Record that no version compatibility check exists ([`e2dbe3a`](https://github.com/1minEpowMinX/SysInfo/commit/e2dbe3aa6c0f331873af19756e00caaf489fbca8))
- Name the collected field after the boot time it carries ([`92ef5f2`](https://github.com/1minEpowMinX/SysInfo/commit/92ef5f21566a71cabdebf01e7017ef4d5c4270d9))
- **ext**: Record why the popup's sizes are absolute ([`76cad14`](https://github.com/1minEpowMinX/SysInfo/commit/76cad142e044749170a5d2e4fceeabcdaf25330d))
- **ext**: Bring the docstrings to the project standard ([`616cf2d`](https://github.com/1minEpowMinX/SysInfo/commit/616cf2d482b03d85b4ae0535fd3b9ad2d3a18f3e))
- **ext**: Move the reasons out of the docstrings ([`96bdf20`](https://github.com/1minEpowMinX/SysInfo/commit/96bdf20f3d4260eafdec8ad01cb9c7cefbf2d87e))
- **api**: Record why the server's member order cannot change ([`9bc3cf4`](https://github.com/1minEpowMinX/SysInfo/commit/9bc3cf4e8125ac5b13bc108342542f400a62e6f2))
- **ui**: Describe the substitution toText actually makes ([`c717e61`](https://github.com/1minEpowMinX/SysInfo/commit/c717e61ac1d23ec0b0dda761de9bdcd07c894915))
- **legal**: Name the Qt modules and the LLVM runtime the delivery ships ([`6cdc06e`](https://github.com/1minEpowMinX/SysInfo/commit/6cdc06e1af6917c7319c47fb50d5aaa213ed433f))
- Point the readmes at this repository and the current release ([`bf4aabc`](https://github.com/1minEpowMinX/SysInfo/commit/bf4aabc2973aad7018f9f2a1a312de3a50885221))
- **security**: Record the configuration the whitelist defaults want ([`dd77c31`](https://github.com/1minEpowMinX/SysInfo/commit/dd77c310f9350a27d227715f22f8fe47d17cc2c6))
- **legal**: Generate the third-party notice from Qt's own SPDX documents ([`8e46ab0`](https://github.com/1minEpowMinX/SysInfo/commit/8e46ab05974ac68b8e8379c7f0209c7e389f6435))
- Refresh the changelog ([`e860202`](https://github.com/1minEpowMinX/SysInfo/commit/e860202957f17c22483240f1fdd68099f5970353))

### Tests

- Cover the onboarding sequence and stop the suite writing real settings ([`3c9418f`](https://github.com/1minEpowMinX/SysInfo/commit/3c9418f5c19a516eb4f5a950c62ea732a764b3fb))
- **sysinfo**: Cover the SMBIOS memory parser with synthetic tables ([`3127a2c`](https://github.com/1minEpowMinX/SysInfo/commit/3127a2c02ebc98f0f03c0398a085c7509492a90f))
- **ext**: Cover the extension against a faked browser ([`063d476`](https://github.com/1minEpowMinX/SysInfo/commit/063d476d2353ea86e49ac143ce28e31698330432))
- **ext**: Close the holes the mutation check found ([`9a3dbf4`](https://github.com/1minEpowMinX/SysInfo/commit/9a3dbf4aa2a403149d126467492ff0fa5fde9bcb))
- **ext**: Reach the branches the poll and the backstop were hiding ([`3748df6`](https://github.com/1minEpowMinX/SysInfo/commit/3748df66ed2793182a4c2e7b69673b3d7a22047e))
- **ext**: Mount the request form the submit watcher looks for ([`dbaf055`](https://github.com/1minEpowMinX/SysInfo/commit/dbaf05518644c002d26c4553e77efd18e933b275))
- **api**: Cover a restart of the integration server ([`0491cb3`](https://github.com/1minEpowMinX/SysInfo/commit/0491cb3492f57bf4d2b23d44e3b70366586f21d9))
- **ext**: Hold the delivery to what the manifest can reach ([`fb290ef`](https://github.com/1minEpowMinX/SysInfo/commit/fb290efc286e673c18b6095b3110f52d3fcf62a5))

### Build

- Bump version to 2.0.1 in manifest files for Chromium and Firefox ([`a357c40`](https://github.com/1minEpowMinX/SysInfo/commit/a357c406a3e966e2c8f6f5c641c9c3e60b67be59))
- **deps-dev**: Bump esbuild ([`706be03`](https://github.com/1minEpowMinX/SysInfo/commit/706be0327489604ce8d22ebf193af57ae742d54b))
- Bump version to 3.1.0 in project and manifest files ([`c3d84d3`](https://github.com/1minEpowMinX/SysInfo/commit/c3d84d37852e8f85b67b4938dd823377c0586f49))
- Split the HTTP endpoint out of the core library ([`3c10b88`](https://github.com/1minEpowMinX/SysInfo/commit/3c10b8897172e9db87f08236a57805c9bc7d99ee))
- **icons**: Generate the platform icon containers from the master SVG ([`a9ebde4`](https://github.com/1minEpowMinX/SysInfo/commit/a9ebde471d7df3c5af485b26c819ab659bd566c8))
- **resources**: Generate the Windows and macOS resource containers ([`a2d2bc3`](https://github.com/1minEpowMinX/SysInfo/commit/a2d2bc3443fd27adc2afb10026e064c380978cc5))
- **tests**: Derive the run_tests roster from the tests directory ([`4285583`](https://github.com/1minEpowMinX/SysInfo/commit/428558319a342a5db9c3d04fc704637a907a6feb))
- **assets**: Add an icon generator driven by the svg master ([`d2caa0d`](https://github.com/1minEpowMinX/SysInfo/commit/d2caa0d1f83f381c7ce5aec8ae45f5e5aadc3fbd))
- **ext**: Bump the extension to 2.1.0 ([`4d8cecb`](https://github.com/1minEpowMinX/SysInfo/commit/4d8cecb3452b569ae9c2e9abf80290f431981072))
- Settle line endings in the repository rather than per clone ([`bde7cce`](https://github.com/1minEpowMinX/SysInfo/commit/bde7cce305b14e103f6160a68cbbb71b10c55ddd))
- **ext**: Read deployment values from a configuration file ([`ce80985`](https://github.com/1minEpowMinX/SysInfo/commit/ce80985eac3236f5ca36044be093796182d2700f))
- **ext**: Generate the browser's copy of the configuration ([`4c9b706`](https://github.com/1minEpowMinX/SysInfo/commit/4c9b7065611116712c5c65b47fcdfcb1291b110c))
- **BREAKING** **ext**: Render each manifest from a shared base and a per-browser overlay ([`689921a`](https://github.com/1minEpowMinX/SysInfo/commit/689921ade640b138413b171b365e6f10582ca349))
- **ext**: Lay out the delivery tree for both browsers in one command ([`b76d8bd`](https://github.com/1minEpowMinX/SysInfo/commit/b76d8bdbbf5cd21821e2d0489e5751335bd52885))

### Operations

- Gate every push on both test suites ([`713e846`](https://github.com/1minEpowMinX/SysInfo/commit/713e84657c930a96a91b1cb178ef54540af98758))
- Render the changelog and the release notes from the commit history ([`e54c73f`](https://github.com/1minEpowMinX/SysInfo/commit/e54c73f106927900bff7d048d7285fca29eab896))
- Drop the Windows build and take the actions off node 20 ([`38853e8`](https://github.com/1minEpowMinX/SysInfo/commit/38853e85237052d36c5ae4ec4e3e22c46ac8ff04))

### Style

- **ext**: Autoformatting ([`1ad5493`](https://github.com/1minEpowMinX/SysInfo/commit/1ad54935dda419580a966ecf913e385b091682ca))
- **ext**: Format JSON configuration for better readability ([`3dffd14`](https://github.com/1minEpowMinX/SysInfo/commit/3dffd14597a581a7304c822de2f37e5cc18b5a91))

### Housekeeping

- Commit the ignore rules instead of keeping them per clone ([`da21df0`](https://github.com/1minEpowMinX/SysInfo/commit/da21df05c398668e0ef4324e18c642cfa2d9d476))
## [3.0.0] - 2026-05-13


### Features

- Add a cross-platform logging system ([`d826ebe`](https://github.com/1minEpowMinX/SysInfo/commit/d826ebe9b73a467b2bbff29bd0d49278282cdc7b))
- Enhance IP address retrieval by adding checks for virtual bridges and VPN interfaces ([`98b6a9c`](https://github.com/1minEpowMinX/SysInfo/commit/98b6a9c7bb7e7f06fe99efe0058fa200d78e2e97))
- Add unit tests for integration server and utility functions ([`935b3ac`](https://github.com/1minEpowMinX/SysInfo/commit/935b3acfb84bab60d26ed14afcdeed3acba08a91))
- Set organization and application details in main function ([`9e07cb3`](https://github.com/1minEpowMinX/SysInfo/commit/9e07cb34a4b61f2d7498530dcfc285b71ab57a1d))
- Ensure syslog is opened before logging on Linux ([`03c7b8f`](https://github.com/1minEpowMinX/SysInfo/commit/03c7b8f89aeccffbd5f1feee77d4e7b2b605cb3e))
- Enhance IntegrationServer with CORS support and cleanup methods ([`83004fa`](https://github.com/1minEpowMinX/SysInfo/commit/83004fae9dfbedc3543070a66cca3ab618897c12))
- Update clipboard functionality to include system info in tray icon tooltip ([`3dd0d89`](https://github.com/1minEpowMinX/SysInfo/commit/3dd0d897514ba276f0c551cfb7a157649c39fdcf))
- Add resource prefix for translations in CMakeLists and clean up resource files ([`651f727`](https://github.com/1minEpowMinX/SysInfo/commit/651f72744253833c8bc3b46e53a2768fd294aca0))
- Add LGPL-3.0 license text and update README with licensing information ([`4aa88d9`](https://github.com/1minEpowMinX/SysInfo/commit/4aa88d91a348a687e18a3ec10a521fce86dd061c))
- Add post-build commands to deploy Qt runtime and copy documentation files ([`257c797`](https://github.com/1minEpowMinX/SysInfo/commit/257c79706b9dd80b83b8b49ec987ee5776bb3d81))
- **security**: Layered request gate for IntegrationServer ([`257b0dc`](https://github.com/1minEpowMinX/SysInfo/commit/257b0dc2049cfc00fdaace4db24b4d72ae10deb2))
- **security**: Add custom header "X-Sysinfo-Client" to fetch requests ([`6deace6`](https://github.com/1minEpowMinX/SysInfo/commit/6deace695031c83ba990a03c7f0cfa54add9d38b))
- **api**: Split /systeminfo payload by caller — extension gets bare data ([`caa1d09`](https://github.com/1minEpowMinX/SysInfo/commit/caa1d098a4266b5a11a7811826a496ac545992be))
- **api**: Add /version endpoint exposing PROJECT_VERSION and BUILD_DATE ([`3962323`](https://github.com/1minEpowMinX/SysInfo/commit/3962323797099850a8073bbbfeda258bc1480808))
- **popup**: Redesign as multi-tab SPA with status, data, history, settings ([`27a9dc2`](https://github.com/1minEpowMinX/SysInfo/commit/27a9dc207cdfb7c70448437364279fcec0cb4b52))
- **bg**: Overhaul service worker — proxyFetch, getVersion, diagPing, permission audit ([`e3bd6f6`](https://github.com/1minEpowMinX/SysInfo/commit/e3bd6f699d4f4232c9df28c87561f536f010d393))

### Fixes

- Update developer name and copyright year in about dialog ([`43bccc6`](https://github.com/1minEpowMinX/SysInfo/commit/43bccc60105c2f70bcdc7b0fa7fa154326203588))
- Fix exit from the app if the response is negative ([`22e129d`](https://github.com/1minEpowMinX/SysInfo/commit/22e129d94585a331cd2148e08002201ba19f1309))
- Pass parent widget to QMovie and UI elements in TrayGuide constructor ([`348703e`](https://github.com/1minEpowMinX/SysInfo/commit/348703ede342c993d60dad56c5bebfa471e605a8))
- Update build date in AboutDialog to reflect correct information ([`d0cf962`](https://github.com/1minEpowMinX/SysInfo/commit/d0cf9627d9d4177d3b157ebf8af1d0880ca60d1b))
- Correct QTimer singleShot syntax for improved functionality ([`4b4c56a`](https://github.com/1minEpowMinX/SysInfo/commit/4b4c56adbe98b7f48b339c168b0e05c9dafb90bd))
- Prevent multiple connections to tray icon messageClicked signal ([`d658681`](https://github.com/1minEpowMinX/SysInfo/commit/d6586811fe54e3222abe5d9cfaa2af45a6b0a51e))
- Update translation resource path format in loadTranslator function ([`452103a`](https://github.com/1minEpowMinX/SysInfo/commit/452103abb274596c4aa113f0ac5073020e0e5d88))
- Replace QSystemSemaphore with QLockFile for single instance management ([`f77fc05`](https://github.com/1minEpowMinX/SysInfo/commit/f77fc05861be38494c05afff78621861768e2895))
- Remove unnecessary UserNotifications framework linkage for macOS ([`2d44cfc`](https://github.com/1minEpowMinX/SysInfo/commit/2d44cfcbd7eab8a5bd88f59206e456edeb7e6704))
- **security**: Close fall-through hole — browser callers must send X-Sysinfo-Client ([`a5c2c36`](https://github.com/1minEpowMinX/SysInfo/commit/a5c2c36025d66b5c2758af5f32657c4a588987fb))
- **security**: Switch context check to Origin-based, fix Firefox preflight ([`da99ba5`](https://github.com/1minEpowMinX/SysInfo/commit/da99ba541d26d50aea422893534efad733619234))
- Import compatibility module in state management ([`6d6e284`](https://github.com/1minEpowMinX/SysInfo/commit/6d6e2846c8bafe0ab7b26c1b4768f77756c8f335))
- **ext**: Fix svgIcon — use innerHTML+firstElementChild instead of DOMParser ([`1dec7f4`](https://github.com/1minEpowMinX/SysInfo/commit/1dec7f4cf109e2b8c0009c228e8fd53bee4af745))

### Security

- **ext**: Harden DOM, messaging, and input handling ([`4f7adde`](https://github.com/1minEpowMinX/SysInfo/commit/4f7added27e95b9c6252ccab3cdd98daaaced6cb))

### Translations

- Re-sync ru_RU/uk_UA translations after refactor ([`6322787`](https://github.com/1minEpowMinX/SysInfo/commit/6322787759f6ff9ef96b2164a34919a7d723b058))
- Refresh .ts <location> after AboutDialog/TrayGuide split ([`223475d`](https://github.com/1minEpowMinX/SysInfo/commit/223475d8e84f8c94dd7962fa4b08f101a73ff44f))
- Expand message catalogue for popup redesign and content refactor ([`5585d94`](https://github.com/1minEpowMinX/SysInfo/commit/5585d94c32b0f3d6ab4a5a0fc4b1224258b09630))
- Update license from LGPL v3.0 to GPL-3.0 in English, Russian and Ukrainian translations ([`0222074`](https://github.com/1minEpowMinX/SysInfo/commit/022207436e921244183ef1c1a63eb1683238ab94))

### Refactoring

- Improve process instance check and enhance translation loading logic ([`7da9be2`](https://github.com/1minEpowMinX/SysInfo/commit/7da9be243c817d3c05c8461bd6761b2d8552b27e))
- Remove QObject inheritance from SettingsManager and simplify constructor ([`ab754cf`](https://github.com/1minEpowMinX/SysInfo/commit/ab754cf1030e10f4d7a0587f944901868c0a5934))
- Simplify App class by removing singleton pattern and initializing members ([`cb59a49`](https://github.com/1minEpowMinX/SysInfo/commit/cb59a49961be2601884e8f34f539f7d1205f5a60))
- Replace singleton SettingManager with injected reference ([`2337a57`](https://github.com/1minEpowMinX/SysInfo/commit/2337a5747636533b5d7468181adcf90b95b25885))
- Decompose App god class to smaller classes: App, TrayController and WelcomeNotifier ([`953bd60`](https://github.com/1minEpowMinX/SysInfo/commit/953bd60253dc28e5d4f835d450018124beda54b3))
- Reorganize source tree into core/services/ui/app layers ([`7d7daf1`](https://github.com/1minEpowMinX/SysInfo/commit/7d7daf17b6ac440ff213fa837b908befcc89b912))
- Split SystemInfo presentation into Utils::Presenter namespace ([`40e74ba`](https://github.com/1minEpowMinX/SysInfo/commit/40e74ba3ba8bb4743cd1a1e0e806c2ea7f2a0122))
- Move fallback localisation from data layer to presenter ([`0003f77`](https://github.com/1minEpowMinX/SysInfo/commit/0003f77df0a4d7988d3d0544696d764a3386ed46))
- Rename Utils -> sysinfo, drop get-prefixes, struct -> Info ([`357935e`](https://github.com/1minEpowMinX/SysInfo/commit/357935ee6ce6d1bb98cd38084dc2f5f47896f0e7))
- Сlarify the comment about working with the sysinfo s_info structure in Linux ([`eb4578d`](https://github.com/1minEpowMinX/SysInfo/commit/eb4578d133e60b0ddeb6c4f898cfecda7428489d))
- Remove the unnecessary comment from the lastBootTime function ([`4680ecc`](https://github.com/1minEpowMinX/SysInfo/commit/4680ecc3f0d3a30754a1dc0b252af065b956eed9))
- **ui**: Tighten SRP in AboutDialog and split fat constructors ([`d337ba5`](https://github.com/1minEpowMinX/SysInfo/commit/d337ba50ac6a0d7d9f5a209a852df9408f51c7d7))
- Update JSDoc comments for clarity and consistency ([`b930bd5`](https://github.com/1minEpowMinX/SysInfo/commit/b930bd5b4036c8272997d78e72f60364cc08fdef))
- **content**: Split content_script.js into lib/ modules ([`ecefda1`](https://github.com/1minEpowMinX/SysInfo/commit/ecefda1680c3bdb0e9094436768a22ef2aeba105))
- **ext**: Convert to ES modules with shared/constants.js ([`13a99d6`](https://github.com/1minEpowMinX/SysInfo/commit/13a99d66db883f69a9da609fba41c5165c3f3e20))

### Documentation

- Rename license.txt to gpl-3.0.txt ([`e8e188c`](https://github.com/1minEpowMinX/SysInfo/commit/e8e188cb2cc92aee8ca90a45a3fbba31c13c93c6))
- Add Doxygen docstrings to remaining public headers and main ([`2dd0976`](https://github.com/1minEpowMinX/SysInfo/commit/2dd097663cdee49c39ce3dee19118d2a27851f5b))
- Add a section on support for custom browser extensions into install guide ([`3a7a5c7`](https://github.com/1minEpowMinX/SysInfo/commit/3a7a5c76fd7e831f8a3c50728f4527916f44b267))
- Update copyright year to 2026 in README files ([`fea53c3`](https://github.com/1minEpowMinX/SysInfo/commit/fea53c3f24bb57246eb4348d9c4947b678cd3b3f))
- Rewrite security model docstring, cover new policy with tests ([`efd7490`](https://github.com/1minEpowMinX/SysInfo/commit/efd74901be3d97bda20c0ff1add6762307b066ec))
- Update license from LGPL V3.0 to GPL-3.0 in documentation. Clarify license use case in readme.txt ([`925d69b`](https://github.com/1minEpowMinX/SysInfo/commit/925d69b4af506bda19b3a4c3c17158150f99a7e1))
- Bump Qt badge version to 6.11.1 in English, Russian, and Ukrainian README files ([`26e1a07`](https://github.com/1minEpowMinX/SysInfo/commit/26e1a07dc0d169e0e192b12b16fe6a4433ac8be9))

### Build

- Extract sysinfo_core static library, drop relative include paths ([`7e49e1e`](https://github.com/1minEpowMinX/SysInfo/commit/7e49e1e45b3c7566e108d60d13a3fb6fac293d92))
- Update license file reference to GPL-3.0.txt in CMakeLists.txt ([`0bcf69f`](https://github.com/1minEpowMinX/SysInfo/commit/0bcf69fc241dae2efb39327ac0f41328b4677143))
- Bump project version to 3.0.0 ([`c2b131f`](https://github.com/1minEpowMinX/SysInfo/commit/c2b131f8c998241c13b06a608f3efa2e426f45c4))
- Remove unnecessary --no-compiler-runtime flag from windeployqt command ([`fb6b37a`](https://github.com/1minEpowMinX/SysInfo/commit/fb6b37a34cf542b87689b623a43e46bfe7426f92))
- **ext**: Add esbuild to bundle content scripts ([`1d0b213`](https://github.com/1minEpowMinX/SysInfo/commit/1d0b2136eefdd8ed8fc417b7e52b18cca0b37836))
- Add new host permissions for jira-t2 in Firefox manifest ([`fdaa226`](https://github.com/1minEpowMinX/SysInfo/commit/fdaa226c82f1e66dcced029a179fdcbb9b2614e6))

### Style

- Clean up comments and improve code readability in integration server ([`d655789`](https://github.com/1minEpowMinX/SysInfo/commit/d65578926691ea15da80fa9249d36d822fa04c2d))
- Add a space before closing brace in the utils namespace ([`3873e73`](https://github.com/1minEpowMinX/SysInfo/commit/3873e73b7a7c191cefa186bbab153ce492e355a4))
- Improve code readability by adding braces to single-line if statements ([`892e505`](https://github.com/1minEpowMinX/SysInfo/commit/892e505f0e649011d065e75782e9abf6d45a4d37))

### Housekeeping

- Update version to 2.1.0 and modify related metadata ([`d4db9ba`](https://github.com/1minEpowMinX/SysInfo/commit/d4db9bae118a7aa5272a01f0749c676b5188985b))
- Update CMake minimum version to 3.30 and improve project structure ([`997733c`](https://github.com/1minEpowMinX/SysInfo/commit/997733ce7652d86efa4290548ee9ca4d6652df29))
- **manifest**: Bump to v2.0.0, wire content lib/ modules, add jira-t2 host ([`75dc110`](https://github.com/1minEpowMinX/SysInfo/commit/75dc110c15735a42f235b86996e8740435b06499))

### Other

- Refactor system information handling and improve function parameters for better readability ([`f84dfe2`](https://github.com/1minEpowMinX/SysInfo/commit/f84dfe288f4833070267d922699cd571a5bdc342))
- Increase the manifest version to 1.0.2 for Chromium and Firefox ([`b4fe303`](https://github.com/1minEpowMinX/SysInfo/commit/b4fe30365b6b9cb623aa8b01c702b81deb16a4e9))
- Update placeholder messages for clarity in English, Russian, and Ukrainian locales ([`b58ebbc`](https://github.com/1minEpowMinX/SysInfo/commit/b58ebbce7d45417dc8763dd78377f201938bf129))
- Remove the placeholder for the system description from the locales and update the logic for inserting system information ([`378732c`](https://github.com/1minEpowMinX/SysInfo/commit/378732c66996b0afffd75f09c197a1e43d749419))
- Increase the manifest version to 1.0.3 for Chromium and Firefox ([`9e55d08`](https://github.com/1minEpowMinX/SysInfo/commit/9e55d088c70082b475f5d2b2883305befa24b787))
## [2.0.0] - 2025-12-26


### Other

- Fix locale names ([`baa4139`](https://github.com/1minEpowMinX/SysInfo/commit/baa4139a07baab8d8b2c791a5754c958478340d9))
- Delete wrong locale naming ([`02872ce`](https://github.com/1minEpowMinX/SysInfo/commit/02872ced1a6936f9760edb49c0ef2d8508f30938))
- Add support for new URLs for Jira and update the check for allowed ticket types ([`6450cbe`](https://github.com/1minEpowMinX/SysInfo/commit/6450cbee1b6d5613ca71e46d0cd95d0ce09d5f72))
- Add comment and fix doctsting in content_script.js:61. Add comment to service_worker.js ([`bb01898`](https://github.com/1minEpowMinX/SysInfo/commit/bb01898aa335cd797f0f9567c2088b7f4bd69f64))
- Rename mozila_manifest.json to firefox_manifest.json and update strict_min_version to 142.0 ([`5392932`](https://github.com/1minEpowMinX/SysInfo/commit/5392932b572ba94aff45fdd993262378404735eb))
- Lower the minimum browser version for compatibility ([`977686f`](https://github.com/1minEpowMinX/SysInfo/commit/977686f61378f2a6e7b139c2007526ff28edefde))
- Increase the version of the manifests from 1.0 to 1.0.0 ([`120eb6d`](https://github.com/1minEpowMinX/SysInfo/commit/120eb6d1f9e2c8d39ab749eb490581dc95356542))
- Replace the separator with a generation function. Move the preparation of system data to a separate function. Change the order of lines in the text. ([`b4b0db2`](https://github.com/1minEpowMinX/SysInfo/commit/b4b0db2b50646a66a9d3db17787598081698df66))
- Increase the manifests version to 1.0.1 for Chromium and Firefox ([`2a083a6`](https://github.com/1minEpowMinX/SysInfo/commit/2a083a6044372e3b6b337be1a02cac7e44d5dc07))
## [1.4.0] - 2025-02-13


### Fixes

- Fix merge ([`3412aa4`](https://github.com/1minEpowMinX/SysInfo/commit/3412aa44768d306168cdbad29fa6d040c691cd48))

### Other

- Delete the main file to prepare a modular implementation ([`afb8f00`](https://github.com/1minEpowMinX/SysInfo/commit/afb8f00a9c7bbe63a2d9e83110cf0f0e33cf863c))
- Delete resources file to prepare a modular implementation ([`fa4ee59`](https://github.com/1minEpowMinX/SysInfo/commit/fa4ee59040eb047ea90183760b0cd147d9286685))
- Implement entry point ([`747ed88`](https://github.com/1minEpowMinX/SysInfo/commit/747ed88a3b8e458de5d3a4c04831ed0d49d62657))
- Create assets folder to load .ico and .png files ([`c80b495`](https://github.com/1minEpowMinX/SysInfo/commit/c80b495f8d320fb5d6cbb8e2cc8860dcbbce6d5c))
- Implement config module ([`b0425b6`](https://github.com/1minEpowMinX/SysInfo/commit/b0425b629db39489433688e10badfc7e5443dea2))
- Implement first run message module ([`43e3160`](https://github.com/1minEpowMinX/SysInfo/commit/43e31602519115e81d6c5ca80e5608df65e59af2))
- Implement system monitoring module ([`80b047f`](https://github.com/1minEpowMinX/SysInfo/commit/80b047f33c3d5d2651a3937dacadf6c3ecad70bc))
- Implement create and manage icon module ([`034c507`](https://github.com/1minEpowMinX/SysInfo/commit/034c507ba8ca23b5c441a34c863a9a11d2a4a297))
- Generate base dependency file ([`1f86608`](https://github.com/1minEpowMinX/SysInfo/commit/1f8660885038e50a6e1978309002d7208bd9652b))
- Specified import of windll ([`5413420`](https://github.com/1minEpowMinX/SysInfo/commit/5413420ac70f00e56cdcaf3626ef85ce515d7dff))
- Specified import of exit ([`66f7c83`](https://github.com/1minEpowMinX/SysInfo/commit/66f7c8385b536f205547738b27c79e0073164de5))
- Change exit function implementation to fix Win 11 freeze ([`4d3f599`](https://github.com/1minEpowMinX/SysInfo/commit/4d3f599b9d877e46fff7277d87cdbfc856f449e0))
- Add comment to except gaierror ([`0a7be8c`](https://github.com/1minEpowMinX/SysInfo/commit/0a7be8c017f14ed1c94bf6f8bbc52645417549dc))
- Change ip monitotang implementation for cisco anyconnect VPN ([`3ea13f9`](https://github.com/1minEpowMinX/SysInfo/commit/3ea13f96592690fe30d9f9e0de85fe92c1697cb3))
- Change monitoring ip for more smooth updates with timings ([`9394045`](https://github.com/1minEpowMinX/SysInfo/commit/93940454441835f4e0e714b453953359389ac7c7))
- Delete unnecessary text from icon.title ([`6afedec`](https://github.com/1minEpowMinX/SysInfo/commit/6afedec090ba27d43087ebee4c8b5b3077c3c179))
- Imporve imports readability ([`131d403`](https://github.com/1minEpowMinX/SysInfo/commit/131d403f737b52989fc285d05d09991005765631))
- Imporve imports readability ([`221bb31`](https://github.com/1minEpowMinX/SysInfo/commit/221bb310f190cbda577beeca684f3745aae0608f))
- Update README.md

Prepare to realese 1.4.0 ([`235d6eb`](https://github.com/1minEpowMinX/SysInfo/commit/235d6ebadf77b13579b0dca13c808cd5b261f054))
- Clear the repository for migration to Qt ([`6a7cf68`](https://github.com/1minEpowMinX/SysInfo/commit/6a7cf68bf87d9e56e3b501a345911ad3b1ef0f7c))
- Init Qt Base ([`124deff`](https://github.com/1minEpowMinX/SysInfo/commit/124deff4c5a64a10ea9dce408c7b5b56696b05ab))
- Create a basic tray application ([`5754e20`](https://github.com/1minEpowMinX/SysInfo/commit/5754e20e789906caa94dd270e77e15fff1edc238))
- Change the project structure. Add hierarchy ([`746d548`](https://github.com/1minEpowMinX/SysInfo/commit/746d548f5c9f550fd3b365fad12513a16516ca5d))
- Change the project structure. Add hierarchy for translation ([`513daae`](https://github.com/1minEpowMinX/SysInfo/commit/513daae964772085ea7443fd4177b44aae110312))
- Add a localization resource for the Ukrainian language ([`82a0dbf`](https://github.com/1minEpowMinX/SysInfo/commit/82a0dbf567a841368d115ab87be2d2a5f316edac))
- Create a utility module for requesting information from a computer ([`189eb2c`](https://github.com/1minEpowMinX/SysInfo/commit/189eb2c49630b5a1f039f6ad3b31278660c57045))
- Integrate the utilities module into the application ([`fedd7b7`](https://github.com/1minEpowMinX/SysInfo/commit/fedd7b7a52c52cf5aed9c799eca00ecf21f300e6))
- Update the list of resources for compilation. Add the Utils module and the Network package ([`afc788c`](https://github.com/1minEpowMinX/SysInfo/commit/afc788cd10f6cf6e61464afe3c49112c5038a509))
- Change LICENSE ([`a61a97f`](https://github.com/1minEpowMinX/SysInfo/commit/a61a97f36c6d158f23580b4c30121029451a51f7))
- Add .rc file for icon and meta info ([`10aa2ce`](https://github.com/1minEpowMinX/SysInfo/commit/10aa2cea89ffad59ee5541b476028397f370bd83))
- Add mutex system to prevent the application copy launching ([`a259513`](https://github.com/1minEpowMinX/SysInfo/commit/a259513b3e8d3b9c0586ec6330f88cb8433e9ffd))
- Implement interface localization into English and Ukrainian ([`fab4484`](https://github.com/1minEpowMinX/SysInfo/commit/fab4484a256122e5229e4273fb03062730722848))
- Refract func structure ([`4983e77`](https://github.com/1minEpowMinX/SysInfo/commit/4983e77b1e5c27ef44d6fa315f1fe358c0d6111c))
- Add module for tray instructions ([`f06a362`](https://github.com/1minEpowMinX/SysInfo/commit/f06a36255d63ad48a2cf8b5b9b9207d124e383b7))
- Refract naming ([`b03bcb8`](https://github.com/1minEpowMinX/SysInfo/commit/b03bcb85e136bd231c6b92505b739a9a974ba101))
- Implement settings module ([`0e3e6d8`](https://github.com/1minEpowMinX/SysInfo/commit/0e3e6d87c2d78fec2dded9e2baabb7973f0d7087))
- Change icon app. Improve design ([`01f8876`](https://github.com/1minEpowMinX/SysInfo/commit/01f887691d79df20c9ff5f96e86ef06ed707ffc1))
- Implement a cross-platform version of the functionality ([`385bc9a`](https://github.com/1minEpowMinX/SysInfo/commit/385bc9a194f54eb61d3ca9864382518865876394))
- Change the file hierarchy for clarity ([`1a45c7e`](https://github.com/1minEpowMinX/SysInfo/commit/1a45c7ecc178faa993c2d6317bc3eb2589f15b3b))
- Change res dir ([`a5dbd8e`](https://github.com/1minEpowMinX/SysInfo/commit/a5dbd8e72a4fca270265e8325c2050894e03bb43))
- Add cross-platform Cmake and change res dir in app load icon ([`23d4d11`](https://github.com/1minEpowMinX/SysInfo/commit/23d4d114304a72426eded9a57b7b2546b6045a3e))
- Fix resources setting for MacOS ([`9d07337`](https://github.com/1minEpowMinX/SysInfo/commit/9d07337c3cf8375db44f42627786543f88691941))
- Change setting name ([`af5e813`](https://github.com/1minEpowMinX/SysInfo/commit/af5e8139366ccb1de96615e8cdb070afe13d4535))
- Refract code. Improve SRP for App module ([`7bbaae3`](https://github.com/1minEpowMinX/SysInfo/commit/7bbaae32bfa99797f7211dcc3744b29900d6cc04))
- Delete generic files ([`b3d35ed`](https://github.com/1minEpowMinX/SysInfo/commit/b3d35ed31cbba72b17c4c2fcd5b77fe74a6ce5ba))
- Move localization resources to others ([`e8bfd24`](https://github.com/1minEpowMinX/SysInfo/commit/e8bfd2427dd74b5a5d63ce57a19225ca37be44bf))
- Implement tray guide for Windows OS. Also add info ballon message for first run. Add context menu item 'About' ([`c2b6d49`](https://github.com/1minEpowMinX/SysInfo/commit/c2b6d49770f08a9a0a5dac97bf5a87c412f787d4))
- Clarify resources structure ([`6fcd9b6`](https://github.com/1minEpowMinX/SysInfo/commit/6fcd9b6c4c06cdb9e0017c77e9390064529eede8))
- Refract window title ([`d611b4c`](https://github.com/1minEpowMinX/SysInfo/commit/d611b4cf220944cd38920dbd76579e398481cee7))
- Refract naming ([`2ce8f9f`](https://github.com/1minEpowMinX/SysInfo/commit/2ce8f9f4224939c62b8abb1ade7a0dc3344250b4))
- Update resources. Refract naming ([`f587126`](https://github.com/1minEpowMinX/SysInfo/commit/f5871261236fbc482b523eb7594235e6fedbf53f))
- Add 'About' module. Update the CMake file ([`9e3ae34`](https://github.com/1minEpowMinX/SysInfo/commit/9e3ae346fede71cfc816e8a51c4e5e7e91a358e9))
- Connect new modules. Update accompanying text ([`882683d`](https://github.com/1minEpowMinX/SysInfo/commit/882683dc39c9104eebb409904d77a3fe7f410783))
- Update localization files ([`fee84c5`](https://github.com/1minEpowMinX/SysInfo/commit/fee84c52f3f45f82f1de7bcc123c8242c236b81e))
- Delete unused file ([`8f1bd4b`](https://github.com/1minEpowMinX/SysInfo/commit/8f1bd4bc04c07a333f7a8ea425e870cf1f191d35))
- Move project files to a subdirectory ([`b274168`](https://github.com/1minEpowMinX/SysInfo/commit/b274168c4bca719dfe582d68b8c415fdda1971a9))
- Add CORS support to the /systeminfo and /status routes ([`cb09746`](https://github.com/1minEpowMinX/SysInfo/commit/cb09746ef8343ed7bfc4dac820b99f15ee6103b5))
- Correct the path to the .gitignore file in CMakeLists.txt ([`a32f9a2`](https://github.com/1minEpowMinX/SysInfo/commit/a32f9a21a80fdf7a46b5a4552a2581fe7016c94f))
- Clear translation files. Change default language to English ([`53d20db`](https://github.com/1minEpowMinX/SysInfo/commit/53d20db458fed014019d9b43eaa2f86d85a2f3b2))
- Refract imports and sort by alphabet Change default language to English ([`9c8eb91`](https://github.com/1minEpowMinX/SysInfo/commit/9c8eb91e7689c0b0004c2de8fffb5a96d99dfc7d))
- Add bold font for emphasis ([`e7525cb`](https://github.com/1minEpowMinX/SysInfo/commit/e7525cb59e831dca9ffda2cf6e06b29f352fc888))
- Remove the 'Help' menu item. Move it to the 'About' item ([`43d4929`](https://github.com/1minEpowMinX/SysInfo/commit/43d4929e707a0c9e60e40725b3691e4459e168f0))
- Update translation. Delete English translation (original language) ([`a68c9f2`](https://github.com/1minEpowMinX/SysInfo/commit/a68c9f24abb0333377774da58a5a465743fe1750))
- Move license and readme files to root project dir ([`23ee56d`](https://github.com/1minEpowMinX/SysInfo/commit/23ee56d0e17a74c0bcae3d9823a88a6f5832405c))
- Fix translation location. Optimize aboutdialog window size ([`f2ad8e2`](https://github.com/1minEpowMinX/SysInfo/commit/f2ad8e2915896109f04aa7439c4074918724a3e0))
- Fix LICENSE in GitHub information ([`898b0ec`](https://github.com/1minEpowMinX/SysInfo/commit/898b0ecfdf2c30ba07abb1931850bdc0f7ddf28a))
- Add a browser extension for integration with Jira SM. Including localization support and manifests for Chrome and Firefox. ([`3c56855`](https://github.com/1minEpowMinX/SysInfo/commit/3c56855b732e44b54c375c5bab46a54cf15612c2))
- Add the LaunchAgent.plist file to automatically launch the SysInfo application on startup. Update Info.plist with the correct formatting ([`2e9291c`](https://github.com/1minEpowMinX/SysInfo/commit/2e9291caea85425142e32d74faa5eb43667fc74e))
- Rename file to Apple standart ([`61c3b6a`](https://github.com/1minEpowMinX/SysInfo/commit/61c3b6a3e3dc74aec1e53f7ed8bcf03d22c7012e))
- Update the description of the SysInfo application and add autostart parameters to sysinfo.desktop ([`4945835`](https://github.com/1minEpowMinX/SysInfo/commit/49458355e3bb85182a053d7f0c8bdbb7421db0f2))
- Correct the path to the executable file in sysinfo.desktop ([`36e6da0`](https://github.com/1minEpowMinX/SysInfo/commit/36e6da0af266d3e02af96e7a56734f8363431926))
- Rework README.md for the new version of the project ([`37e4424`](https://github.com/1minEpowMinX/SysInfo/commit/37e442462906ed5087a9c97de270f16f62353fc2))
- Fix the link to the documentation in README.md ([`3db53e1`](https://github.com/1minEpowMinX/SysInfo/commit/3db53e1773b0d2cc611a51f1c6e4e94a34ea59b2))
- Change the author name format in README.md ([`63dc531`](https://github.com/1minEpowMinX/SysInfo/commit/63dc5318d6a46ee4308d62acc2ab09ccac82e12b))
- Add an empty line to ‘2. Extract the archive’ to improve readability in README.md. ([`474e9c2`](https://github.com/1minEpowMinX/SysInfo/commit/474e9c2be55475f68fea97253862dcaa0dc1fecd))
- Update notification display time. Create a display delay to wait for full system initialization ([`5a81124`](https://github.com/1minEpowMinX/SysInfo/commit/5a81124452b0cc971c34bfc572505c1ed1fe5ddf))
- Refract source naming. Specifically, add missing definitions and change the use of ico in createTrayIcon to png for better cross-platform compatibility ([`46c63ba`](https://github.com/1minEpowMinX/SysInfo/commit/46c63ba5692277484d25fed5e03ee1c2bd4743b7))
- Fix path after refract naming ([`f350ed8`](https://github.com/1minEpowMinX/SysInfo/commit/f350ed83a5e6867d58708f01037e22de0fe9d110))
- Delete SysInfo/resources.qrc.autosave ([`696eecd`](https://github.com/1minEpowMinX/SysInfo/commit/696eecd69fe5e1296ef749eb215f676ad97d780a))
- Fix renaming ([`0c287c2`](https://github.com/1minEpowMinX/SysInfo/commit/0c287c2a94df5db55f03e924c1104081197cc0c4))
- Create a directory for additional documentation. Create README.txt and LICENSE.txt versions for the build ([`d03e36c`](https://github.com/1minEpowMinX/SysInfo/commit/d03e36c26728fafa8d11f30ef3d34571282e2f89))
- Update README.md: add localization information and update installation instructions for new versions ([`e7e51a7`](https://github.com/1minEpowMinX/SysInfo/commit/e7e51a7e43a2d2ec875a032bed6ad8954b7d817f))
- Add localization: create README in Russian and Ukrainian, update English README ([`9fc0188`](https://github.com/1minEpowMinX/SysInfo/commit/9fc0188c48b9070b7d0c3cd50d4397ee916927e4))
- Update installation instructions in README files for Linux and Windows ([`a5c1227`](https://github.com/1minEpowMinX/SysInfo/commit/a5c12272ea3c9524729608bdd8a62d9d60b6ac53))
- Correct the unpacking example for Windows to a specific path ([`e628381`](https://github.com/1minEpowMinX/SysInfo/commit/e62838181225033905f38076ac77dcbf1488800d))
- Update instructions for extracting archives for Linux and macOS in README files in English, Russian, and Ukrainian ([`6c6e67c`](https://github.com/1minEpowMinX/SysInfo/commit/6c6e67c7af9d7fe99a37d3b48da176dff99e7a3c))
- Correct the path for unpacking macOS archives in README for all languages ([`9eee119`](https://github.com/1minEpowMinX/SysInfo/commit/9eee119090f5816abc620d1e3cb90be5243c707a))
- Update README.md ([`e3ebc05`](https://github.com/1minEpowMinX/SysInfo/commit/e3ebc058a3c3f79748b5765be77497268a5069a6))
- Update README.md ([`1b6a5b7`](https://github.com/1minEpowMinX/SysInfo/commit/1b6a5b71454227cdf68bcfdc4369d2fae344afa7))
## [1.3.2] - 2025-01-13


### Other

- Updated text message from first run app ([`5baedba`](https://github.com/1minEpowMinX/SysInfo/commit/5baedbac747f7279db9c047a0ebd58244bf6a8d5))
- Edited clipboard function format ([`38b1607`](https://github.com/1minEpowMinX/SysInfo/commit/38b1607c76a7be8a4119bab7e21ae782c5e6265a))
- Added necessary resource for instructions ([`0b2d424`](https://github.com/1minEpowMinX/SysInfo/commit/0b2d424ab5b82aec7c735f3e8aa2baf2e3b348b2))
## [1.3.1] - 2025-01-09


### Other

- Update README.md ([`fe2e3d9`](https://github.com/1minEpowMinX/SysInfo/commit/fe2e3d934ccfbf9d838cb933364f779f49a481cd))
- New implementation of data storage and addition of output confirmation ([`d349a1e`](https://github.com/1minEpowMinX/SysInfo/commit/d349a1e7e28bb00ed000d064be2dba4712f5a6e7))
- Optimized resource download process ([`c6480be`](https://github.com/1minEpowMinX/SysInfo/commit/c6480beb8b48af57b7aa7d32b5907149583823aa))
- Update README.md ([`e589fd2`](https://github.com/1minEpowMinX/SysInfo/commit/e589fd2dba9afec5bed23210c69da52f01b9f42e))
- Update README.md ([`f205ef8`](https://github.com/1minEpowMinX/SysInfo/commit/f205ef8d0bcd1cf8efef2173915e4561d013f080))
- Update README.md ([`9d55292`](https://github.com/1minEpowMinX/SysInfo/commit/9d5529260aabc96cc7b55aa16da860e0b065bf79))
## [1.3.0] - 2024-12-02


### Other

- Update README.md ([`53fb747`](https://github.com/1minEpowMinX/SysInfo/commit/53fb747137a54db6f1789492d3557c5ba4c42734))
- Update README.md ([`9fba2ce`](https://github.com/1minEpowMinX/SysInfo/commit/9fba2cef9db324ad639f9041c3b77aecdb664842))
- Update README.md ([`74788eb`](https://github.com/1minEpowMinX/SysInfo/commit/74788eb0953df26ab649450e62b91273f396c15c))
- Added .bat script to clear tray cache in case icons are not working properly ([`5b96b25`](https://github.com/1minEpowMinX/SysInfo/commit/5b96b253bebd47cd74f2609d16f05f823be325a9))
- Update README.md ([`7fd0cb2`](https://github.com/1minEpowMinX/SysInfo/commit/7fd0cb29e6322e3898014ef3010f8d4a65315b84))
- Added an introductory instruction when starting the program for the first time. Revised module import and memory consumption ([`2145d81`](https://github.com/1minEpowMinX/SysInfo/commit/2145d81a4e8079e6d15358c6c1418dbfdabc6b41))
## [1.2.1] - 2024-11-28


### Other

- Update README.md ([`2714238`](https://github.com/1minEpowMinX/SysInfo/commit/2714238e24f2194765047e0578befe416e24035a))
- Update README.md ([`b7f766a`](https://github.com/1minEpowMinX/SysInfo/commit/b7f766afdb60f9b7472e0d8652cf6ebcf21babe7))
- Update README.md ([`ce2ccea`](https://github.com/1minEpowMinX/SysInfo/commit/ce2cceac7222404cc05b2898a0f06821af13e43d))
- Deleted unnecessary imports ([`05b24d1`](https://github.com/1minEpowMinX/SysInfo/commit/05b24d143d7cf7e6a673f01100849e23c1a06cd4))
- Update README.md ([`6f96f8a`](https://github.com/1minEpowMinX/SysInfo/commit/6f96f8a004b4faf92c66f61f434f1c3d0f9ee042))
- Implemented full UK localization ([`5635595`](https://github.com/1minEpowMinX/SysInfo/commit/5635595645f62c93b0c0a07d6b4035c318d624b1))
- Replace .ico image ([`e3a7153`](https://github.com/1minEpowMinX/SysInfo/commit/e3a71536c5547ff8c5524ac64d235f4b6e933d48))
- Replace ,ico path ([`785cd7d`](https://github.com/1minEpowMinX/SysInfo/commit/785cd7d88401b48788131f9325ceba05f56ad949))
## [1.2.0] - 2024-11-27


### Other

- Fixed lock file deletion ([`4b53837`](https://github.com/1minEpowMinX/SysInfo/commit/4b53837280f87c8e78f14b6a9ff34b2ddb0dc45b))
- A different way of getting rid of the lock file has been implemented ([`b69eae9`](https://github.com/1minEpowMinX/SysInfo/commit/b69eae986bfdde242ab4df59b868f6e6e5ba541c))
- Deleted lock_file const ([`c32c35c`](https://github.com/1minEpowMinX/SysInfo/commit/c32c35c6c6ef7fd591d59923aa188241fe230541))
- Update README.md ([`fc4f658`](https://github.com/1minEpowMinX/SysInfo/commit/fc4f65849642a03a2329c677c7043bd880b7d91b))
## [1.1.0] - 2024-11-27


### Other

- Added "Copy to Clipboard" menu item ([`06e4b3e`](https://github.com/1minEpowMinX/SysInfo/commit/06e4b3efe5dad56e4fca4c83c20c7c91af990ab2))
- Added lock-file implementation ([`e23a756`](https://github.com/1minEpowMinX/SysInfo/commit/e23a756102732fa079160e3ee5c33694a355d54a))
- Replaced the tkinter library with a lighter analog ([`8e2a53c`](https://github.com/1minEpowMinX/SysInfo/commit/8e2a53c6098fda80c002f88eccbc223da1d9476f))
- Added main function. The organization of the code has been changed. ([`b6e9d0e`](https://github.com/1minEpowMinX/SysInfo/commit/b6e9d0ec167238ea94da62b566689761e170451d))
- Deleted useless line ([`bcaf76f`](https://github.com/1minEpowMinX/SysInfo/commit/bcaf76f8dffd1eece21d296d6a16d9060b2749f3))
## [1.0.0] - 2024-11-27


### Other

- Init comment ([`268c58d`](https://github.com/1minEpowMinX/SysInfo/commit/268c58de6228978b8cf7920d6dd35afe53209037))
- Create README.md ([`d07cdad`](https://github.com/1minEpowMinX/SysInfo/commit/d07cdad6e223c5dc13d8cb1ebef87a02451897e7))
- Create LICENSE ([`33efd6d`](https://github.com/1minEpowMinX/SysInfo/commit/33efd6d4cad66bdd8e75533e0266ef6b2bb79812))
- Update README.md ([`92c7e87`](https://github.com/1minEpowMinX/SysInfo/commit/92c7e87a4820e22069598248fbb2d7aa51f35bae))
- Update README.md ([`c1367ed`](https://github.com/1minEpowMinX/SysInfo/commit/c1367edd7ee67952ae2338dd4060ec0c4a99ca47))
- Update README.md ([`bb549e3`](https://github.com/1minEpowMinX/SysInfo/commit/bb549e3a368380b7c2626965c3920b51f7815b59))
- Update README.md ([`ebc81eb`](https://github.com/1minEpowMinX/SysInfo/commit/ebc81eb7acccd870ac0612aac4c7e1af45036141))
- Update README.md ([`6253992`](https://github.com/1minEpowMinX/SysInfo/commit/6253992e72bdc963f8efa2aa5325399cec0e465e))

