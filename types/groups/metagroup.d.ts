/// <reference path="../shared.d.ts" />
/// <reference path="./groupmembers.d.ts" />
/// <reference path="./groupinfo.d.ts" />

declare module 'libsession_util_nodejs' {
  type ConstructorOptions = {
    userEd25519Secretkey: Uint8Array;
    groupEd25519Pubkey: Uint8Array;
    groupEd25519Secretkey: Uint8Array | null;
    dumpedInfo: Uint8Array | null;
    dumpedMembers: Uint8Array | null;
    dumpedKeys: Uint8Array | null;
  };

  type MetaGroupWrapper = GroupInfoWrapper &
    GroupMemberWrapper & {
      init: (options: ConstructorOptions) => void;

      // shared actions
      needsPush: () => boolean;

      // info

      // members

      // keys
      keysNeedsRekey: () => boolean;
    };

  export type MetaGroupWrapperActionsCalls = MakeWrapperActionCalls<MetaGroupWrapper>;

  export class MetaGroupWrapperNode {
    constructor(ConstructorOptions);

    // shared actions
    public needsPush: MetaGroupWrapper['needsPush'];

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
    public memberSetInvited: MetaGroupWrapper['memberSetInvited'];
    public memberErase: MetaGroupWrapper['memberErase'];

    // keys

    public keysNeedsRekey: MetaGroupWrapper['keysNeedsRekey'];
  }

  export type MetaGroupActionsType =
    | ['init', ConstructorOptions]
    // shared actions
    | MakeActionCall<MetaGroupWrapper, 'needsPush'>

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
    | MakeActionCall<MetaGroupWrapper, 'memberSetInvited'>
    | MakeActionCall<MetaGroupWrapper, 'memberErase'>

    // keys actions
    | MakeActionCall<MetaGroupWrapper, 'keysNeedsRekey'>;
}
