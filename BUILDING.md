# Building instructions

Note: Keep in mind that when doing a build from the folder directly using `yarn configure` and `yarn build`, you get a node native module which is not compatible with the embedded node of electron.
To get a build compatible with electron, you need to rebuilt the native dependencies of you application with `yarn postinstall` from the root `session-desktop` directory.

## Linux build

This should be enough:

    yarn configure && yarn build

## Windows build

Having installed Microsft visual studio 2022 with the Desktop dev tools (at least), you then need to force node-gyp to find the right path for it with:

    npm config edit

And paste:
msvs-version=2022
msbuild-path=C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe

Without those changes, `node-gyp` keeps looking for a msbuild in `Program Files (x86)` which does not exist as the 2022 version only comes in the x64 architecture.
