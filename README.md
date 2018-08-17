This is just a silly experiment I did to learn my way around the X-Plane SDK.  It was inspired by a joke from a YouTube flight simmer that he wished a plugin reminded him to log into his virtual airline because he always forgot.  So this plugin will wait until all engines on the aircraft are running and then pop up a one time reminder.

For those that wish to build this themselves, first you need to download the [X-Plane SDK](https://developer.x-plane.com/sdk/plugin-sdk-downloads/) version 3.0.1 or higher and put it into the XPlaneSDK folder in the repo (it should contain CHeaders, Libraries, etc directory).

After that, just install CMake and run the following commands:

```bash
# Linux / Mac
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j8
```

```powershell
# Windows
mkdir build
cd build
cmake -G "Visual Studio 15 2017 Win64" .. # Or another version of Visual Studio if you wish
msbuild LogInDummy.sln # Or open with Visual Studio
```

After that you should be able to find (in the Release directory) a file called either LogInDummy.xpl (Windows) or libLogInDummy.xpl (Linux / Mac).  This can go into your X-Plane Resources/Plugins/LogInDummy folder under the appropriate directory (e.g. win_x64/LogInDummy.xpl).  
