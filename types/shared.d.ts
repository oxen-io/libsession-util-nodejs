declare module 'libsession_util_nodejs' {
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

  type MakeActionCall<A extends RecordOfFunctions, B extends keyof A> = [B, ...Parameters<A[B]>];

  /**
   *
   * Base Config wrapper logic
   *
   */

  export type BaseConfigWrapper = {
    needsDump: () => boolean;
    needsPush: () => boolean;
    push: () => PushConfigResult;
    dump: () => Uint8Array;
    confirmPushed: (seqno: number, hash: string) => void;
    merge: (toMerge: Array<{ hash: string; data: Uint8Array }>) => number;
    storageNamespace: () => number;
    currentHashes: () => Array<string>;
  };

  export type BaseConfigFunctions =
    | 'needsDump'
    | 'needsPush'
    | 'push'
    | 'dump'
    | 'confirmPushed'
    | 'merge'
    | 'needsPush'
    | 'storageNamespace'
    | 'currentHashes';

  export type GenericWrapperActionsCall<A extends string, B extends keyof BaseConfigWrapper> = (
    wrapperId: A,
    ...args: Parameters<BaseConfigWrapper[B]>
  ) => Promise<ReturnType<BaseConfigWrapper[B]>>;

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

  export type GroupPubkeyType = `03${string}`; // type of a string which starts by the 03 prefixed used for closed group

  type MakeGroupActionCall<A extends BaseConfigWrapper, B extends keyof A> = [
    B,
    ...Parameters<A[B]>
  ]; // all of the groupActionCalls need the pubkey of the group we are targetting

  type AsyncGroupWrapper<T extends (...args: any) => any> = (
    groupPk: GroupPubkeyType,
    ...args: Parameters<T>
  ) => Promise<ReturnType<T>>;

  type MakeGroupWrapperActionCalls<Type extends RecordOfFunctions> = {
    [Property in keyof Omit<Type, 'initGroup'>]: AsyncGroupWrapper<Type[Property]>;
  };

  export type PriorityType = { priority: number }; // -1 means hidden, 0 means normal, > 1 means pinned
  type GroupSubWrapperType = 'GroupInfo' | 'GroupMember' | 'GroupKeys';

  type GroupInfoShared = {
    name: string | null;
    createdAtSeconds: number | null;
    deleteAttachBeforeSeconds: number | null;
    deleteBeforeSeconds: number | null;
    expirySeconds: number | null;
    profilePicture: ProfilePicture | null;
  };

  export type GroupInfoGet = GroupInfoShared & {
    isDestroyed: boolean;
    secretKey?: Uint8Array;
  };
}
