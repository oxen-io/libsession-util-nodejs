/// <reference path="../shared.d.ts" />

declare module 'libsession_util_nodejs' {
  /**
   *
   * Group Wrapper Logics
   */
  export type GroupInfoSet = GroupInfoShared;

  type GroupInfoWrapper = BaseConfigWrapper & {
    initGroup: (
      ed25519Pubkey: Uint8Array,
      secretKey: Uint8Array | null,
      dump: Uint8Array | null
    ) => void;

    // GroupInfo related methods
    getInfo: () => GroupInfoGet | null;
    setInfo: (info: GroupInfoSet) => GroupInfoGet;
    destroy: () => void;
  };

  export type GroupInfoWrapperActionsCalls = MakeGroupWrapperActionCalls<GroupInfoWrapper> & {
    initGroup: (
      ed25519Pubkey: GroupPubkeyType,
      secretKey: Uint8Array | null,
      dump: Uint8Array | null
    ) => Promise<void>;
  };

  export class GroupInfoWrapperNode extends BaseConfigWrapperNode {
    constructor(
      ed25519Pubkey: GroupPubkeyType,
      secretKey: Uint8Array | null,
      dump: Uint8Array | null
    );

    // GroupInfo related methods
    public getInfo: GroupInfoWrapper['getInfo'];
    public setInfo: GroupInfoWrapper['setInfo'];
    public destroy: GroupInfoWrapper['destroy'];
  }

  type GroupActionMap<T extends BaseConfigWrapper> = Omit<
    { [K in keyof T]: MakeGroupActionCall<T, K> },
    BaseConfigFunctions
  >;

  type GroupActionUnion<T extends BaseConfigWrapper> = GroupActionMap<T>[keyof GroupActionMap<T>];

  export type GroupInfoActionsType = GroupActionUnion<GroupInfoWrapper>;
}
