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
    /**
     * - 0 means not removed,
     * - 1 means removed (libsession: REMOVED_MEMBER),
     * - 2 means removed with messages  (libsession: REMOVED_MEMBER_AND_MESSAGES)
     */
    removedStatus: number;
    promoted: boolean;
    admin: boolean;
  };

  type GroupMemberWrapper = {
    // GroupMember related methods
    memberGet: (pubkeyHex: PubkeyType) => GroupMemberGet | null;
    memberGetOrConstruct: (pubkeyHex: PubkeyType) => GroupMemberGet;
    memberGetAll: () => Array<GroupMemberGet>;
    memberGetAllPendingRemovals: () => Array<GroupMemberGet>;

    // setters
    memberSetName: (pubkeyHex: PubkeyType, newName: string) => void;
    memberSetInvited: (pubkeyHex: PubkeyType, failed: boolean) => void;
    memberSetPromoted: (pubkeyHex: PubkeyType, failed: boolean) => void;
    memberSetAdmin: (pubkeyHex: PubkeyType) => void;
    memberSetAccepted: (pubkeyHex: PubkeyType) => void;
    memberSetProfilePicture: (
      pubkeyHex: PubkeyType,
      profilePicture: ProfilePicture
    ) => void;
    membersMarkPendingRemoval: (members: Array<PubkeyType>, withMessages: boolean) => void;

    // eraser
    memberEraseAndRekey: (members: Array<PubkeyType>) => boolean;
  };
}
