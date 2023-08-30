/// <reference path="./shared.d.ts" />

declare module 'libsession_util_nodejs' {
  /**
   *
   * Group Wrapper Logics
   */

  type GroupInfoShared = {
    name: string | null;
    createdAtSeconds: number | null;
    deleteAttachBeforeSeconds: number | null;
    deleteBeforeSeconds: number | null;
    expirySeconds: number | null;
    profilePicture: ProfilePicture | null;
  };

  export type GroupInfoGet = GroupInfoShared & {
    isDestroyed: boolean;
  };

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

  /**
   *
   * GroupMembers wrapper logic
   *
   */
  type GroupMemberShared = {
    pubkeyHex: string;
    name: string | null;
    profilePicture: ProfilePicture | null;
  };

  export type GroupMemberGet = GroupMemberShared & {
    invitePending: boolean;
    inviteFailed: boolean;
    promotionPending: boolean;
    promotionFailed: boolean;
    promoted: boolean;
  };

  type GroupMemberWrapper = BaseConfigWrapper & {
    initGroup: (
      ed25519Pubkey: Uint8Array,
      secretKey: Uint8Array | null,
      dump: Uint8Array | null
    ) => void;

    // GroupMember related methods
    get: (pubkeyHex: string) => GroupMemberGet | null;
    getOrConstruct: (pubkeyHex: string) => GroupMemberGet;
    getAll: () => Array<GroupMemberGet>;

    // setters
    setName: (pubkeyHex: string, newName: string) => GroupMemberGet;
    setInvited: (pubkeyHex: string, failed: boolean) => GroupMemberGet;
    setPromoted: (pubkeyHex: string, failed: boolean) => GroupMemberGet;
    setAccepted: (pubkeyHex: string) => GroupMemberGet;
    setProfilePicture: (pubkeyHex: string, profilePicture: ProfilePicture) => GroupMemberGet;

    // eraser
    erase: (pubkeyHex: string) => null;
  };

  export type GroupMemberWrapperActionsCalls = MakeGroupWrapperActionCalls<GroupMemberWrapper> & {
    initGroup: (
      ed25519Pubkey: GroupPubkeyType,
      secretKey: Uint8Array | null,
      dump: Uint8Array | null
    ) => Promise<void>;
  };
}
