/// <reference path="../shared.d.ts" />
/// <reference path="./usergroups.d.ts" />

declare module 'libsession_util_nodejs' {
  export type ConvoVolatileType = '1o1' | UserGroupsType;

  export type BaseConvoInfoVolatile = {
    lastRead: number; // defaults to 0, unixTimestamp in ms
    unread: boolean; // defaults to false
  };

  type ConvoInfoVolatile1o1 = BaseConvoInfoVolatile & { pubkeyHex: string };
  type ConvoInfoVolatileLegacyGroup = BaseConvoInfoVolatile & { pubkeyHex: string };
  type ConvoInfoVolatileGroup = BaseConvoInfoVolatile & { pubkeyHex: GroupPubkeyType };
  type ConvoInfoVolatileCommunity = BaseConvoInfoVolatile & CommunityDetails;

  // type ConvoInfoVolatileCommunity = BaseConvoInfoVolatile & { pubkeyHex: string }; // we need a `set` with the full url but maybe not for the `get`

  type ConvoInfoVolatileWrapper = BaseConfigWrapper & {
    init: (secretKey: Uint8Array, dump: Uint8Array | null) => void;
    /** This function is used to free wrappers from memory only */
    free: () => void;

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

    // group related methods
    getGroup: (pubkeyHex: GroupPubkeyType) => ConvoInfoVolatileGroup | null;
    getAllGroups: () => Array<ConvoInfoVolatileGroup>;
    setGroup: (pubkeyHex: GroupPubkeyType, lastRead: number, unread: boolean) => void;
    eraseGroup: (pubkeyHex: GroupPubkeyType) => boolean;

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
    | MakeActionCall<ConvoInfoVolatileWrapper, 'free'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'get1o1'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'getAll1o1'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'set1o1'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'erase1o1'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'getLegacyGroup'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'getAllLegacyGroups'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'setLegacyGroup'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'eraseLegacyGroup'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'getGroup'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'getAllGroups'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'setGroup'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'eraseGroup'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'getCommunity'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'setCommunityByFullUrl'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'getAllCommunities'>
    | MakeActionCall<ConvoInfoVolatileWrapper, 'eraseCommunityByFullUrl'>;
}
