/// <reference path="../shared.d.ts" />

declare module 'libsession_util_nodejs' {
  type ConstructorOptions = {
    userEd25519Secretkey: Uint8Array;
    groupEd25519Pubkey: Uint8Array;
    groupEd25519Secretkey: Uint8Array | null;
    dumpedInfo: Uint8Array | null;
    dumpedMembers: Uint8Array | null;
    dumpedKeys: Uint8Array | null;
  };

  type MetaGroupWrapper = {
    init: (options: ConstructorOptions) => void;

    // info
    infoNeedsPush: () => boolean;

    // members
    membersNeedsPush: () => boolean;

    // keys
    keysNeedsRekey: () => boolean;
  };

  export type MetaGroupWrapperActionsCalls = MakeWrapperActionCalls<MetaGroupWrapper>;

  export class MetaGroupWrapperNode {
    constructor(ConstructorOptions);
    // members

    public infoNeedsPush: MetaGroupWrapper['infoNeedsPush'];

    // members

    public membersNeedsPush: MetaGroupWrapper['membersNeedsPush'];

    // keys

    public keysNeedsRekey: MetaGroupWrapper['keysNeedsRekey'];
  }

  export type MetaGroupActionsType =
    | ['init', ConstructorOptions]
    | MakeActionCall<MetaGroupWrapper, 'infoNeedsPush'>
    | MakeActionCall<MetaGroupWrapper, 'membersNeedsPush'>
    | MakeActionCall<MetaGroupWrapper, 'keysNeedsRekey'>;
}
