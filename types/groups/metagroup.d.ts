/// <reference path="../shared.d.ts" />
/// <reference path="./groupmembers.d.ts" />
/// <reference path="./groupinfo.d.ts" />
/// <reference path="./groupkeys.d.ts" />

declare module 'libsession_util_nodejs' {
  export type ConfirmKeysPush = [data: Uint8Array, hash: string, timestampMs: number];

  export type GroupWrapperConstructor = {
    userEd25519Secretkey: Uint8Array; // (len 64)
    groupEd25519Pubkey: Uint8Array; // the ed25519 pubkey without the 03 prefix  (len 32)
    groupEd25519Secretkey: Uint8Array | null; // the ed25519 privkey if we have it (len 64) (comes from usergroup wrapper if we have it)
    metaDumped: Uint8Array | null;
  };

  type MetaGroupWrapper = GroupInfoWrapper &
    GroupMemberWrapper &
    GroupKeysWrapper & {
      // shared actions
      init: (options: GroupWrapperConstructor) => void;
      needsPush: () => boolean;
      push: () => {
        groupInfo: PushConfigResult | null;
        groupMember: PushConfigResult | null;
        groupKeys: PushKeyConfigResult | null;
      };
      needsDump: () => boolean;
      metaDump: () => Uint8Array;
      metaMakeDump: () => Uint8Array;
      metaConfirmPushed: ({
        groupInfo,
        groupMember,
      }: {
        groupInfo: ConfirmPush | null;
        groupMember: ConfirmPush | null;
      }) => void;
      metaMerge: ({
        groupInfo,
        groupKeys,
        groupMember,
      }: {
        groupInfo: Array<MergeSingle> | null;
        groupMember: Array<MergeSingle> | null;
        groupKeys: Array<MergeSingle & { timestampMs: number }> | null;
      }) => void;
    };

  // this just adds an argument of type GroupPubkeyType in front of the parameters of that function
  type AddGroupPkToFunction<T extends (...args: any) => any> = (
    ...args: [GroupPubkeyType, ...Parameters<T>]
  ) => ReturnType<T>;

  export type MetaGroupWrapperActionsCalls = MakeWrapperActionCalls<{
    [key in keyof MetaGroupWrapper]: AddGroupPkToFunction<MetaGroupWrapper[key]>;
  }>;

  export class MetaGroupWrapperNode {
    constructor(options: GroupWrapperConstructor);

    // shared actions
    public needsPush: MetaGroupWrapper['needsPush'];
    public push: MetaGroupWrapper['push'];
    public needsDump: MetaGroupWrapper['needsDump'];
    public metaDump: MetaGroupWrapper['metaDump'];
    public metaMakeDump: MetaGroupWrapper['metaMakeDump'];
    public metaConfirmPushed: MetaGroupWrapper['metaConfirmPushed'];
    public metaMerge: MetaGroupWrapper['metaMerge'];
    public currentHashes: MetaGroupWrapper['currentHashes'];

    // info
    public infoGet: MetaGroupWrapper['infoGet'];
    public infoSet: MetaGroupWrapper['infoSet'];
    public infoDestroy: MetaGroupWrapper['infoDestroy'];

    // members
    public memberGet: MetaGroupWrapper['memberGet'];
    public memberGetOrConstruct: MetaGroupWrapper['memberGetOrConstruct'];
    public memberGetAll: MetaGroupWrapper['memberGetAll'];
    public memberSetAccepted: MetaGroupWrapper['memberSetAccepted'];
    public memberSetName: MetaGroupWrapper['memberSetName'];
    public memberSetPromoted: MetaGroupWrapper['memberSetPromoted'];
    public memberSetAdmin: MetaGroupWrapper['memberSetAdmin'];
    public memberSetInvited: MetaGroupWrapper['memberSetInvited'];
    public memberEraseAndRekey: MetaGroupWrapper['memberEraseAndRekey'];
    public memberSetProfilePicture: MetaGroupWrapper['memberSetProfilePicture'];

    // keys

    public keysNeedsRekey: MetaGroupWrapper['keysNeedsRekey'];
    public keyRekey: MetaGroupWrapper['keyRekey'];
    public loadKeyMessage: MetaGroupWrapper['loadKeyMessage'];
    public encryptMessage: MetaGroupWrapper['encryptMessage'];
    public decryptMessage: MetaGroupWrapper['decryptMessage'];
    public makeSwarmSubAccount: MetaGroupWrapper['makeSwarmSubAccount'];
    public swarmSubaccountSign: MetaGroupWrapper['swarmSubaccountSign'];
  }

  export type MetaGroupActionsType =
    | ['init', GroupWrapperConstructor]
    // shared actions
    | MakeActionCall<MetaGroupWrapper, 'needsPush'>
    | MakeActionCall<MetaGroupWrapper, 'push'>
    | MakeActionCall<MetaGroupWrapper, 'needsDump'>
    | MakeActionCall<MetaGroupWrapper, 'metaDump'>
    | MakeActionCall<MetaGroupWrapper, 'metaMakeDump'>
    | MakeActionCall<MetaGroupWrapper, 'metaConfirmPushed'>
    | MakeActionCall<MetaGroupWrapper, 'metaMerge'>

    // info actions
    | MakeActionCall<MetaGroupWrapper, 'infoGet'>
    | MakeActionCall<MetaGroupWrapper, 'infoSet'>
    | MakeActionCall<MetaGroupWrapper, 'infoDestroy'>

    // member actions
    | MakeActionCall<MetaGroupWrapper, 'memberGet'>
    | MakeActionCall<MetaGroupWrapper, 'memberGetOrConstruct'>
    | MakeActionCall<MetaGroupWrapper, 'memberGetAll'>
    | MakeActionCall<MetaGroupWrapper, 'memberSetAccepted'>
    | MakeActionCall<MetaGroupWrapper, 'memberSetName'>
    | MakeActionCall<MetaGroupWrapper, 'memberSetPromoted'>
    | MakeActionCall<MetaGroupWrapper, 'memberSetAdmin'>
    | MakeActionCall<MetaGroupWrapper, 'memberSetInvited'>
    | MakeActionCall<MetaGroupWrapper, 'memberEraseAndRekey'>
    | MakeActionCall<MetaGroupWrapper, 'memberSetProfilePicture'>

    // keys actions
    | MakeActionCall<MetaGroupWrapper, 'keysNeedsRekey'>
    | MakeActionCall<MetaGroupWrapper, 'keyRekey'>
    | MakeActionCall<MetaGroupWrapper, 'loadKeyMessage'>
    | MakeActionCall<MetaGroupWrapper, 'currentHashes'>
    | MakeActionCall<MetaGroupWrapper, 'encryptMessage'>
    | MakeActionCall<MetaGroupWrapper, 'decryptMessage'>
    | MakeActionCall<MetaGroupWrapper, 'makeSwarmSubAccount'>
    | MakeActionCall<MetaGroupWrapper, 'swarmSubaccountSign'>
    | MakeActionCall<MetaGroupWrapper, 'generateSupplementKeys'>
    | MakeActionCall<MetaGroupWrapper, 'swarmSubAccountToken'>
    | MakeActionCall<MetaGroupWrapper, 'swarmVerifySubAccount'>;
}
