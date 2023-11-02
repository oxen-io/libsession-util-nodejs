/// <reference path="../shared.d.ts" />

declare module 'libsession_util_nodejs' {
  /**
   *
   * GroupMembers wrapper logic
   *
   */
  type GroupMemberShared = {
    pubkeyHex: PubkeyType;
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

  type GroupMemberWrapper = {
    // GroupMember related methods
    memberGet: (pubkeyHex: PubkeyType) => GroupMemberGet | null;
    memberGetOrConstruct: (pubkeyHex: PubkeyType) => GroupMemberGet;
    memberGetAll: () => Array<GroupMemberGet>;

    // setters
    memberSetName: (pubkeyHex: PubkeyType, newName: string) => GroupMemberGet;
    memberSetInvited: (pubkeyHex: PubkeyType, failed: boolean) => GroupMemberGet;
    memberSetPromoted: (pubkeyHex: PubkeyType, failed: boolean) => GroupMemberGet;
    memberSetAccepted: (pubkeyHex: PubkeyType) => GroupMemberGet;
    memberSetProfilePicture: (
      pubkeyHex: PubkeyType,
      profilePicture: ProfilePicture
    ) => GroupMemberGet;

    // eraser
    memberErase: (members: Array<PubkeyType>) => boolean;
  };
}
