* Getting Started *

1. Clone this repo
2. Clone submodules if you didn't already
3. Prepare libsamplerate (below)
4. Open `StretchFix/StretchFix.xcodeproj`

** libsamplerate setup **
libsamplerate needs the Xcode Command Line Tools, autoconf, and friends...

1. `sudo xcode-select --install`
2. `brew install autoconf`
3. `brew install automake`
4. `brew install libtool`
5. `brew install pkg-config`
6. Finally, in `libs/libsamplerate`, run `./autogen.sh`
7. Run `make all`

* Install/Run *
Once you successfully build the StretchFix target and all its Target Dependencies, you must install the Audio Units.
Copy the products `sscau.component` and `secretrabbitau.component` to `~/Library/Audio/Plug-Ins/Components/`
You can run `auval -s auol` to see if the plugs in are present. This lists the "Offline" types of AudioUnits
You should see
```
auol srvs YSPf  -  StretchFix: SecretRabbitAU
auol ssca YSPf  -  StretchFix: SSCAU
```
in the list somewhere.

If you modify these AudioUnits, you'll have to re-install them to see the changes at runtime. An enterprising hacker could probably make a post-build step to install these.
