declare module 'libsession_util_nodejs' {
  /**
   *
   * Utilities
   *
   */

  type AsyncWrapper<T extends (...args: any) => any> = (
    ...args: Parameters<T>
  ) => Promise<ReturnType<T>>;

  export type RecordOfFunctions = Record<string, (...args: any) => any>;

  type MakeWrapperActionCalls<Type extends RecordOfFunctions> = {
    [Property in keyof Type]: AsyncWrapper<Type[Property]>;
  };

  export type ProfilePicture = {
    url: string | null;
    key: Uint8Array | null;
  };

  export type PushConfigResult = { data: Uint8Array; seqno: number; hashes: Array<string> };
  export type MergeSingle = { hash: string; data: Uint8Array };

  type MakeActionCall<A extends RecordOfFunctions, B extends keyof A> = [B, ...Parameters<A[B]>];

  /**
   *
   * Base Config wrapper logic
   *
   */

  type BaseConfigWrapper = {
    needsDump: () => boolean;
    needsPush: () => boolean;
    push: () => PushConfigResult;
    dump: () => Uint8Array;
    confirmPushed: (seqno: number, hash: string) => void;
    merge: (toMerge: Array<MergeSingle>) => Array<string>; // merge returns the array of hashes that merged correctly
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

  export type ConstantsType = {
    /** 100 bytes */
    CONTACT_MAX_NAME_LENGTH: number;
    /** 100 bytes - for legacy groups and communities */
    BASE_GROUP_MAX_NAME_LENGTH: number;
    /** 100 bytes */
    GROUP_INFO_MAX_NAME_LENGTH: number;
    /** 411 bytes
     *
     * BASE_URL_MAX_LENGTH + '/r/' + ROOM_MAX_LENGTH + qs_pubkey.size() + hex pubkey + null terminator
     */
    COMMUNITY_FULL_URL_MAX_LENGTH: number;
  };

  export const CONSTANTS: ConstantsType;
}
