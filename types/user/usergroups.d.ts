/// <reference path="../shared.d.ts" />

declare module 'libsession_util_nodejs' {
  /**
   *
   * UserGroups wrapper logic
   *
   */

  export type UserGroupsType = 'Community' | 'LegacyGroup' | 'Group';

  export type CommunityDetails = {
    fullUrlWithPubkey: string;
    baseUrl: string;
    roomCasePreserved: string;
  };

  export type CommunityInfo = CommunityDetails &
    PriorityType & {
      pubkeyHex: string;
    };

  export type LegacyGroupMemberInfo = {
    pubkeyHex: string;
    isAdmin: boolean;
  };

  type BaseUserGroup = PriorityType & {
    joinedAtSeconds: number; // equivalent to the lastJoinedTimestamp in Session desktop but in seconds rather than MS
  };

  export type LegacyGroupInfo = BaseUserGroup & {
    pubkeyHex: string; // The legacy group "session id" (33 bytes).
    name: string; // human-readable; this should normally always be set, but in theory could be set to an empty string.
    encPubkey: Uint8Array; // bytes (32 or empty)
    encSeckey: Uint8Array; // bytes (32 or empty)
    // disappearingTimerSeconds: number; // in seconds, 0 == disabled.
    members: Array<LegacyGroupMemberInfo>;
  };

  type UserGroupsGet = BaseUserGroup & {
    pubkeyHex: GroupPubkeyType; // The group "session id" (33 bytes), starting with 03.
    secretKey: Uint8Array | null; // len 64
    authData: Uint8Array | null;
    name: string | null;
  };

  /**
   * We can set anything on a UserGroup and can omit fields by explicitely setting them to null.
   * The only one which cannot be omited is the pubkeyHex
   */
  type UserGroupsSet = Pick<UserGroupsGet, 'pubkeyHex'> &
    AllFieldsNullable<Omit<UserGroupsGet, 'pubkeyHex'>>;

  type UserGroupsWrapper = BaseConfigWrapper & {
    init: (secretKey: Uint8Array, dump: Uint8Array | null) => void;

    // Communities related methods
    /** Note: can have the pubkey argument set or not.  */
    getCommunityByFullUrl: (fullUrlWithOrWithoutPubkey: string) => CommunityInfo | null;

    /**
     * This will take care of duplicates and just return the existing one if one is already there matching it.
     * Note: this needs the pubkey to be provided in the argument as it might need to create it.
     */
    setCommunityByFullUrl: (fullUrlWithPubkey: string, priority: number) => null;
    getAllCommunities: () => Array<CommunityInfo>;

    /**
     * Note: can have the pubkey argument set or not.
     */
    eraseCommunityByFullUrl: (fullUrlWithOrWithoutPubkey: string) => void;
    buildFullUrlFromDetails: (baseUrl: string, roomId: string, pubkeyHex: string) => string;

    // Legacy groups related methods
    getLegacyGroup: (pubkeyHex: string) => LegacyGroupInfo | null;
    getAllLegacyGroups: () => Array<LegacyGroupInfo>;
    setLegacyGroup: (info: LegacyGroupInfo) => boolean;
    eraseLegacyGroup: (pubkeyHex: string) => boolean;

    // Groups related methods
    // the create group always returns the secretKey as we've just created it
    createGroup: () => UserGroupsGet & NonNullable<Pick<UserGroupsGet, 'secretKey'>>;
    getGroup: (pubkeyHex: GroupPubkeyType) => UserGroupsGet | null;
    getAllGroups: () => Array<UserGroupsGet>;
    setGroup: (info: UserGroupsSet) => UserGroupsGet;
    eraseGroup: (pubkeyHex: GroupPubkeyType) => boolean;
  };

  export type UserGroupsWrapperActionsCalls = MakeWrapperActionCalls<UserGroupsWrapper>;

  export class UserGroupsWrapperNode extends BaseConfigWrapperNode {
    constructor(secretKey: Uint8Array, dump: Uint8Array | null);
    // communities related methods
    public getCommunityByFullUrl: UserGroupsWrapper['getCommunityByFullUrl'];
    public setCommunityByFullUrl: UserGroupsWrapper['setCommunityByFullUrl'];
    public getAllCommunities: UserGroupsWrapper['getAllCommunities'];
    public eraseCommunityByFullUrl: UserGroupsWrapper['eraseCommunityByFullUrl'];
    public buildFullUrlFromDetails: UserGroupsWrapper['buildFullUrlFromDetails'];

    // legacy-groups related methods
    public getLegacyGroup: UserGroupsWrapper['getLegacyGroup'];
    public getAllLegacyGroups: UserGroupsWrapper['getAllLegacyGroups'];
    public setLegacyGroup: UserGroupsWrapper['setLegacyGroup'];
    public eraseLegacyGroup: UserGroupsWrapper['eraseLegacyGroup'];

    // groups related methods
    public createGroup: UserGroupsWrapper['createGroup'];
    public getGroup: UserGroupsWrapper['getGroup'];
    public getAllGroups: UserGroupsWrapper['getAllGroups'];
    public setGroup: UserGroupsWrapper['setGroup'];
    public eraseGroup: UserGroupsWrapper['eraseGroup'];
  }

  export type UserGroupsConfigActionsType =
    | ['init', Uint8Array, Uint8Array | null]
    | MakeActionCall<UserGroupsWrapper, 'getCommunityByFullUrl'>
    | MakeActionCall<UserGroupsWrapper, 'setCommunityByFullUrl'>
    | MakeActionCall<UserGroupsWrapper, 'getAllCommunities'>
    | MakeActionCall<UserGroupsWrapper, 'eraseCommunityByFullUrl'>
    | MakeActionCall<UserGroupsWrapper, 'buildFullUrlFromDetails'>
    | MakeActionCall<UserGroupsWrapper, 'getAllLegacyGroups'>
    | MakeActionCall<UserGroupsWrapper, 'getLegacyGroup'>
    | MakeActionCall<UserGroupsWrapper, 'setLegacyGroup'>
    | MakeActionCall<UserGroupsWrapper, 'eraseLegacyGroup'>
    | MakeActionCall<UserGroupsWrapper, 'createGroup'>
    | MakeActionCall<UserGroupsWrapper, 'getGroup'>
    | MakeActionCall<UserGroupsWrapper, 'getAllGroups'>
    | MakeActionCall<UserGroupsWrapper, 'setGroup'>
    | MakeActionCall<UserGroupsWrapper, 'eraseGroup'>;
}
