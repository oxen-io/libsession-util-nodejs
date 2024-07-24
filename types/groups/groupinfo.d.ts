/// <reference path="../shared.d.ts" />

declare module 'libsession_util_nodejs' {
  export type GroupInfoWrapper = {
    // GroupInfo related methods
    infoGet: () => GroupInfoGet;
    infoSet: (info: GroupInfoSet) => GroupInfoGet;
    infoDestroy: () => void;
  };
}
