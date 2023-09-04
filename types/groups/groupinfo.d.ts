/// <reference path="../shared.d.ts" />

declare module 'libsession_util_nodejs' {
  /**
   *
   * Group Wrapper Logics
   */
  export type GroupInfoSet = GroupInfoShared;

  export type GroupInfoWrapper = {
    // GroupInfo related methods
    infoGet: () => GroupInfoGet | null;
    infoSet: (info: GroupInfoSet) => GroupInfoGet;
    infoDestroy: () => void;
  };
}
