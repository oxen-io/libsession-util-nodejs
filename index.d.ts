declare module 'libsession_util_nodejs' {
  /**
   * This is quite a complex setup, but so far I didn't find an easier way to describe what we want to do with the wrappers and have strong typings.
   *
   * Essentially, each of the wrappers have 2 sides
   *  - one side are calls made by the webworker directly to the wrapper
   *  - the other side are calls made by the renderer to the webworker (which should forward them to the wrapper)
   *
   * We cannot pass unserializable data between those two, so we need to have a serializable way of calling one
   * method of a wrapper with the required arguments.
   * Those serializable data, are `UserConfigActionsType` or just any of the `*ActionsType`. They are defined with a tuple of what each methods accepts on which wrapper with which argument.
   *
   * Then, we need to define the side of what the worker can call directly on the wrapper & the calls to be made from the renderer to the webworker.
   * To avoid too much duplication, I've created a third Type, which is reused in the other two.
   * The `ContactsWrapper` or `UserConfigWrapper` is that third Type (for each wrapper).
   * The `*InsideWorker` is the part reusing that Type to enable the webworker to do its actions directly.
   * The `*WrapperActionsCalls` is the part which can be called from the renderer to do actions through the webworker on the wrappers.
   *
   *
   *
   * If at some point we get to make all of those type derived from `ContactsWrapper` or `UserConfigWrapper` it would be nice. Typescript is probably already able to do it, with some pain.
   * For instance, it would be nice to the `*ActionsType` derived from the ContactsWrapper too, but I've already spent too much time trying to get it working.
   *
   *
   */

  // region Utilities
  
  type AsyncWrapper<T extends (...args: any) => any> = (
    ...args: Parameters<T>
  ) => Promise<ReturnType<T>>;

  type MakeWrapperActionCalls<Type extends BaseConfigWrapper> = {
    [Property in keyof Type]: AsyncWrapper<Type[Property]>;
  };

  export type ProfilePicture = {
    url: string | null;
    key: Uint8Array | null;
  };

  export type PushConfigResult = { data: Uint8Array; seqno: number; hashes: Array<string> };

  type MakeActionCall<A extends BaseConfigWrapper, B extends keyof A> = [B, ...Parameters<A[B]>];

  // endregion

  // region Base Config wrapper logic
  

  type BaseConfigWrapper = {
    needsDump: () => boolean;
    needsPush: () => boolean;
    push: () => PushConfigResult;
    dump: () => Uint8Array;
    confirmPushed: (seqno: number, hash: string) => void;
    merge: (toMerge: Array<{ hash: string; data: Uint8Array }>) => number;
    storageNamespace: () => number;
    currentHashes: () => Array<string>;
  };

  export type BaseConfigActions =
    | MakeActionCall<BaseConfigWrapper, 'needsDump'>
    | MakeActionCall<BaseConfigWrapper, 'needsPush'>
    | MakeActionCall<BaseConfigWrapper, 'push'>
    | MakeActionCall<BaseConfigWrapper, 'dump'>
    | MakeActionCall<BaseConfigWrapper, 'confirmPushed'>
    | MakeActionCall<BaseConfigWrapper, 'merge'>
    | MakeActionCall<BaseConfigWrapper, 'storageNamespace'>
    | MakeActionCall<BaseConfigWrapper, 'currentHashes'>;

  export abstract class BaseConfigWrapperNode {
    public needsDump: BaseConfigWrapper['needsDump'];
    public needsPush: BaseConfigWrapper['needsPush'];
    public push: BaseConfigWrapper['push'];
    public dump: BaseConfigWrapper['dump'];
    public confirmPushed: BaseConfigWrapper['confirmPushed'];
    public merge: BaseConfigWrapper['merge'];
    public storageNamespace: BaseConfigWrapper['storageNamespace'];
    public currentHashes: BaseConfigWrapper['currentHashes'];
  }

  export type BaseWrapperActionsCalls = MakeWrapperActionCalls<BaseConfigWrapper>;

  // endregion

  // region User config wrapper logic

  type UserConfigWrapper = BaseConfigWrapper & {
    init: (secretKey: Uint8Array, dump: Uint8Array | null) => void;
    getUserInfo: () => {
      name: string;
      priority: number;
      url: string;
      key: Uint8Array;
    };
    setUserInfo: (
      name: string,
      priority: number,
      profilePic: { url: string; key: Uint8Array } | null
    ) => void;
    setEnableBlindedMsgRequest: (msgRequest: boolean) => void;
    getEnableBlindedMsgRequest: () => boolean | undefined;
    setExpiry: (expirySeconds: number) => void;
    getExpiry: () => number | undefined;
  };

  export type UserConfigWrapperActionsCalls = MakeWrapperActionCalls<UserConfigWrapper>;

  /**
   * To be used inside the web worker only (calls are synchronous and won't work asynchrously)
   */
  export class UserConfigWrapperNode extends BaseConfigWrapperNode {
    constructor(secretKey: Uint8Array, dump: Uint8Array | null);
    public getUserInfo: UserConfigWrapper['getUserInfo'];
    public setUserInfo: UserConfigWrapper['setUserInfo'];
    public getEnableBlindedMsgRequest: UserConfigWrapper['getEnableBlindedMsgRequest'];
    public setEnableBlindedMsgRequest: UserConfigWrapper['setEnableBlindedMsgRequest'];
    public getExpiry: UserConfigWrapper['getExpiry'];
    public setExpiry: UserConfigWrapper['setExpiry'];
  }

  /**
   * Those actions are used internally for the web worker communication.
   * You should never need to import them in Session directly
   * You will need to add an entry here if you add a new function
   */
  export type UserConfigActionsType =
    | ['init', Uint8Array, Uint8Array | null]
    | MakeActionCall<UserConfigWrapper, 'getUserInfo'>
    | MakeActionCall<UserConfigWrapper, 'setUserInfo'>
    | MakeActionCall<UserConfigWrapper, 'getEnableBlindedMsgRequest'>
    | MakeActionCall<UserConfigWrapper, 'setEnableBlindedMsgRequest'>
    | MakeActionCall<UserConfigWrapper, 'getExpiry'>
    | MakeActionCall<UserConfigWrapper, 'setExpiry'>;

  // endregion

  // region Contacts wrapper logic


  type ContactsWrapper = BaseConfigWrapper & {
    init: (secretKey: Uint8Array, dump: Uint8Array | null) => void;
    get: (pubkeyHex: string) => ContactInfo | null;
    set: (contact: ContactInfoSet) => void;
    getAll: () => Array<ContactInfo>;
    erase: (pubkeyHex: string) => void;
  };

  export type ContactsWrapperActionsCalls = MakeWrapperActionCalls<ContactsWrapper>;

  type ContactInfoShared = {
    id: string;
    name?: string;
    nickname?: string;
    profilePicture?: ProfilePicture;
    priority: number; // -1 means hidden, 0 means normal, > 1 means pinned
    createdAtSeconds: number; // can only be set the first time a contact is created, a new change won't overide the value in the wrapper.

    // expirationMode: 'off' | 'disappearAfterRead' | 'disappearAfterSend'; // the same as defined in the disappearingBranch
    // expirationTimerSeconds: number;
  };

  export type ContactInfoSet = ContactInfoShared & {
    approved?: boolean;
    approvedMe?: boolean;
    blocked?: boolean;
  };

  export type ContactInfo = ContactInfoShared & {
    approved: boolean;
    approvedMe: boolean;
    blocked: boolean;
  };

  export class ContactsConfigWrapperNode extends BaseConfigWrapperNode {
    constructor(secretKey: Uint8Array, dump: Uint8Array | null);
    public get: ContactsWrapper['get'];
    public set: ContactsWrapper['set'];
    public getAll: ContactsWrapper['getAll'];
    public erase: ContactsWrapper['erase'];
  }

  export type ContactsConfigActionsType =
    | ['init', Uint8Array, Uint8Array | null]
    | MakeActionCall<ContactsWrapper, 'get'>
    | MakeActionCall<ContactsWrapper, 'set'>
    | MakeActionCall<ContactsWrapper, 'getAll'>
    | MakeActionCall<ContactsWrapper, 'erase'>;

  // endregion

  // region UserGroups wrapper logic


  export type UserGroupsType = 'Community' | 'LegacyGroup';

  export type CommunityDetails = {
    fullUrlWithPubkey: string;
    baseUrl: string;
    roomCasePreserved: string;
  };

  export type CommunityInfo = CommunityDetails & {
    pubkeyHex: string;
    priority: number; // -1 means hidden, 0 means normal, > 0 means pinned. We currently don't support hidden communities on the client though
  };

  export type LegacyGroupMemberInfo = {
    pubkeyHex: string;
    isAdmin: boolean;
  };

  export type LegacyGroupInfo = {
    pubkeyHex: string; // The legacy group "session id" (33 bytes).
    name: string; // human-readable; this should normally always be set, but in theory could be set to an empty string.
    encPubkey: Uint8Array; // bytes (32 or empty)
    encSeckey: Uint8Array; // bytes (32 or empty)
    // disappearingTimerSeconds: number; // in seconds, 0 == disabled.
    priority: number; // -1 means hidden, 0 means normal, > 1 means pinned. We currently don't support hidden groups on the client though
    members: Array<LegacyGroupMemberInfo>;
    joinedAtSeconds: number; // equivalent to the lastJoinedTimestamp in Session desktop but in seconds rather than MS
  };

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
    | MakeActionCall<UserGroupsWrapper, 'eraseLegacyGroup'>;

  // endregion

  // region Conversation Volatile wrapper logic
  
  export type ConvoVolatileType = '1o1' | UserGroupsType;

  export type BaseConvoInfoVolatile = {
    lastRead: number; // defaults to 0, unixTimestamp in ms
    unread: boolean; // defaults to false
  };

  type ConvoInfoVolatile1o1 = BaseConvoInfoVolatile & { pubkeyHex: string };
  type ConvoInfoVolatileLegacyGroup = BaseConvoInfoVolatile & { pubkeyHex: string };
  type ConvoInfoVolatileCommunity = BaseConvoInfoVolatile & CommunityDetails;

  // type ConvoInfoVolatileCommunity = BaseConvoInfoVolatile & { pubkeyHex: string }; // we need a `set` with the full url but maybe not for the `get`

  type ConvoInfoVolatileWrapper = BaseConfigWrapper & {
    init: (secretKey: Uint8Array, dump: Uint8Array | null) => void;

    // 1o1 related methods
    get1o1: (pubkeyHex: string) => ConvoInfoVolatile1o1 | null;
    getAll1o1: () => Array<ConvoInfoVolatile1o1>;
    set1o1: (pubkeyHex: string, lastRead: number, unread: boolean) => void;
    erase1o1: (pubkeyHex: string) => void;

    // legacy group related methods
    getLegacyGroup: (pubkeyHex: string) => ConvoInfoVolatileLegacyGroup | null;
    getAllLegacyGroups: () => Array<ConvoInfoVolatileLegacyGroup>;
    setLegacyGroup: (pubkeyHex: string, lastRead: number, unread: boolean) => void;
    eraseLegacyGroup: (pubkeyHex: string) => boolean;

    // communities related methods
    getCommunity: (communityFullUrl: string) => ConvoInfoVolatileCommunity | null; // pubkey not required
    getAllCommunities: () => Array<ConvoInfoVolatileCommunity>;
    setCommunityByFullUrl: (fullUrlWithPubkey: string, lastRead: number, unread: boolean) => void;
    eraseCommunityByFullUrl: (fullUrlWithOrWithoutPubkey: string) => void;
  };

  export type ConvoInfoVolatileWrapperActionsCalls =
    MakeWrapperActionCalls<ConvoInfoVolatileWrapper>;

  export class ConvoInfoVolatileWrapperNode extends BaseConfigWrapperNode {
    constructor(secretKey: Uint8Array, dump: Uint8Array | null);
    // 1o1 related methods
    public get1o1: ConvoInfoVolatileWrapper['get1o1'];
    public getAll1o1: ConvoInfoVolatileWrapper['getAll1o1'];
    public set1o1: ConvoInfoVolatileWrapper['set1o1'];
    public erase1o1: ConvoInfoVolatileWrapper['eraseLegacyGroup'];

    // legacy-groups related methods
    public getLegacyGroup: ConvoInfoVolatileWrapper['getLegacyGroup'];
    public getAllLegacyGroups: ConvoInfoVolatileWrapper['getAllLegacyGroups'];
    public setLegacyGroup: ConvoInfoVolatileWrapper['setLegacyGroup'];
    public eraseLegacyGroup: ConvoInfoVolatileWrapper['eraseLegacyGroup'];

    // communities related methods
    public getCommunity: ConvoInfoVolatileWrapper['getCommunity'];
    public setCommunityByFullUrl: ConvoInfoVolatileWrapper['setCommunityByFullUrl'];
    public getAllCommunities: ConvoInfoVolatileWrapper['getAllCommunities'];
    public eraseCommunityByFullUrl: ConvoInfoVolatileWrapper['eraseCommunityByFullUrl'];
  }

  export type ConvoInfoVolatileConfigActionsType =
    | ['init', Uint8Array, Uint8Array | null]
    | MakeActionCall<ConvoInfoVolatileWrapper, 'get1o1'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'getAll1o1'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'set1o1'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'erase1o1'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'getLegacyGroup'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'getAllLegacyGroups'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'setLegacyGroup'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'eraseLegacyGroup'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'getCommunity'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'setCommunityByFullUrl'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'getAllCommunities'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'eraseCommunityByFullUrl'>;
  // endregion
}
