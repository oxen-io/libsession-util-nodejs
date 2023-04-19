```
pip install git-archive-all
PACKAGE_VERSION=$(cat package.json | grep version | head -1 | awk -F: '{ print $2 }' | sed 's/[", ]//g')
git-archive-all libsession_util_nodejs-v$PACKAGE_VERSION.tar.gz
```

And then upload this to the new release you are drafting on github.
