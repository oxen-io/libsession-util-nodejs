## Libsession-util NodeJS

This library is the wrappers around libsession-util for NodeJS. It is built using the [node-addon-api](https://github.com/nodejs/node-addon-api) and [cmake-js](https://github.com/cmake-js/cmake-js). The build and dev instructions are quite custom, so check them out below.

### Issue with pnpm

The yarn package manager (used on session-desktop) does understand what is a git url in a dependency as part of a package.json file, but won't do a full clone including submodules when forking such a repository. For more details, see https://github.com/yarnpkg/yarn/issues/1488

We tried using `cmake-js` and node's `preinstall scripts` to make this happen, but with the different platforms (linux, windows and macOS) having their own issues it turned out to be a nightmare.

Having lost enough time on this for now, I've decided to instead of relying of git submodules being fetched before the compilation can happen, I would generate a release tarball which includes the git modules and add that tarball dependency in the package.json of session-desktop.

### Dev instructions

Clone this project to somewhere **not** part of `session-desktop` node_modules:

```
cd [FOLDER_NOT_IN_SESSION_DESKTOP]
git clone --recursive git@github.com:oxen-io/libsession-util-nodejs.git
```

Always do your changes in `[FOLDER_NOT_IN_SESSION_DESKTOP]/libsession-util-nodejs`, never in the one under session-desktop's `node_modules` as you might override your local changes.
Then, you can quickly compile a non-electron build by running `pnpm cmake-js` from that folder. This is a quick incremental build which can check for C++ compilation errors.
Once your changes are ready to be tested in the `session-desktop` you can compile an electron build using this command:

```
cd [SESSION_DESKTOP_PATH]; rm -rf node_modules/libsession_util_nodejs; cp -R [THIS_PROJECT_PATH] node_modules/libsession_util_nodejs; cd node_modules/libsession_util_nodejs && rm -rf build && pnpm install && cd [SESSION_DESKTOP_PATH] && pnpm build:workers
```

Replace `[SESSION_DESKTOP_PATH]` with the full path to your `session-desktop` folder, replace `[THIS_PROJECT_PATH]` with the path to the root of this project folder.

Every part of this command is needed and might need to be updated using your paths. Also, the `worker:libsession` needs to be recompiled too to include the just created .node file in itself. This is done by the `pnpm build:workers` command.

Note: The `electron` property in the `config` object will need to be updated in the `package.json` every time we update `electron` package in [session-desktop](https://github.com/oxen-io/session-desktop/) so that the versions match. It is a node version, but not part of the official node docs. If you compiled the node module for an incorrect electron/node version you will get an error on `session-desktop` start.

### Making a Release and updating Session-desktop

First, make sure all your changes are commited and pushed to the `libsession-util-nodejs` project from your `[FOLDER_NOT_IN_SESSION_DESKTOP]` folder.
Then, bump the version in the package.json of the nodejs wrapper.

- A **patch** version bump is required only if you have changed the implementation of an existing function or doing a hot fix for libsession version used by `session-desktop`.

- A **minor** version bump is required if you have added a new function or changed the signature of an existing one.

Then, run these commands:

```
# bump the version in the package.json
git add .
git commit
git push upstream main
# Make sure you do the steps above, otherwise the tag won't be on the right commit with the `gh release create`

pip install git-archive-all
PACKAGE_VERSION=$(cat package.json | grep version | head -1 | awk -F: '{ print $2 }' | sed 's/[", ]//g')
git-archive-all libsession_util_nodejs-v$PACKAGE_VERSION.tar.gz
gh release create v$PACKAGE_VERSION -t v$PACKAGE_VERSION --latest --generate-notes
gh release upload v$PACKAGE_VERSION libsession_util_nodejs-v$PACKAGE_VERSION.tar.gz
```

And then upload the generated `libsession_util_nodejs-v$PACKAGE_VERSION.tar.gz` to that release just created on github.
The git-archive-all relies on the `.gitattributes` to know what to include or exclude.

Once this is done, update the dependency on `session-desktop`.
Make sure to remove the existing one first (with the include `pnpm remove` below) as you might have messed up your `node_modules` doing the dev instructions.

```
pnpm remove libsession_util_nodejs && pnpm add https://github.com/oxen-io/libsession-util-nodejs/releases/download/v0.1.15/libsession_util_nodejs-v0.1.15.tar.gz
```

Keep in mind that you need to update the two version numbers (e.g. `0.1.15`) to the just created release version of this project.

The cmake-js script will be run on the pnpm add and should recompile your latest release of the `libsession-util-nodejs` as part of the build process and as part of the github actions on `session-desktop`.
You most likely need to recompile the workers to include those new changes: `pnpm worker:utils && pnpm worker:libsession`.

You should have your changes in the next release!
