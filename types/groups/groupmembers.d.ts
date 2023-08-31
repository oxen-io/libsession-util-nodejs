/// <reference path="../shared.d.ts" />

declare module 'libsession_util_nodejs' {
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
