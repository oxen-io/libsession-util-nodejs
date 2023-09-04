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

  type GroupMemberWrapper = {
    // GroupMember related methods
    memberGet: (pubkeyHex: string) => GroupMemberGet | null;
    memberGetOrConstruct: (pubkeyHex: string) => GroupMemberGet;
    memberGetAll: () => Array<GroupMemberGet>;

    // setters
    memberSetName: (pubkeyHex: string, newName: string) => GroupMemberGet;
    memberSetInvited: (pubkeyHex: string, failed: boolean) => GroupMemberGet;
    memberSetPromoted: (pubkeyHex: string, failed: boolean) => GroupMemberGet;
    memberSetAccepted: (pubkeyHex: string) => GroupMemberGet;
    memberSetProfilePicture: (pubkeyHex: string, profilePicture: ProfilePicture) => GroupMemberGet;

    // eraser
    memberErase: (pubkeyHex: string) => null;
  };
}
