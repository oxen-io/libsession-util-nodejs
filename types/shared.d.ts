declare module 'libsession_util_nodejs' {
  type Uint8ArrayFixedLength<T extends number> = {
    buffer: Uint8Array;
    length: T;
  };

  /**
   * Allow a single type to be Nullable. i.e. string => string | null
   */
  type Nullable<T> = T | null;

  /**
   * Allow all the fields of a type to be -themselves- nullable.
   * i.e. {field1: string, field2: number} => {field1: string | null, field2: number | null}
   */
  type AllFieldsNullable<T> = {
    [P in keyof T]: Nullable<T[P]>;
  };

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

  export type PushConfigResult = {
    data: Uint8Array;
    seqno: number;
    hashes: Array<string>;
    namespace: number;
  };

  export type PushKeyConfigResult = Pick<PushConfigResult, 'data' | 'namespace'>;

  export type ConfirmPush = [seqno: number, hash: string];
  export type MergeSingle = { hash: string; data: Uint8Array };

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
    makeDump: () => Uint8Array;
    confirmPushed: (seqno: number, hash: string) => void;
    merge: (toMerge: Array<MergeSingle>) => Array<string>; // merge returns the array of hashes merged correctly
    currentHashes: () => Array<string>;
  };

  export type BaseConfigFunctions =
    | 'needsDump'
    | 'needsPush'
    | 'push'
    | 'dump'
    | 'makeDump'
    | 'confirmPushed'
    | 'merge'
    | 'needsPush'
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
    | MakeActionCall<BaseConfigWrapper, 'makeDump'>
    | MakeActionCall<BaseConfigWrapper, 'confirmPushed'>
    | MakeActionCall<BaseConfigWrapper, 'merge'>
    | MakeActionCall<BaseConfigWrapper, 'currentHashes'>;

  export abstract class BaseConfigWrapperNode {
    public needsDump: BaseConfigWrapper['needsDump'];
    public needsPush: BaseConfigWrapper['needsPush'];
    public push: BaseConfigWrapper['push'];
    public dump: BaseConfigWrapper['dump'];
    public makeDump: BaseConfigWrapper['makeDump'];
    public confirmPushed: BaseConfigWrapper['confirmPushed'];
    public merge: BaseConfigWrapper['merge'];
    public currentHashes: BaseConfigWrapper['currentHashes'];
  }

  export type BaseWrapperActionsCalls = MakeWrapperActionCalls<BaseConfigWrapper>;

  export type GroupPubkeyType = `03${string}`; // type of a string which starts by the 03 prefixed used for closed group
  export type PubkeyType = `05${string}`; // type of a string which starts by the 05 prefixed used for **legacy** closed group and session ids
  export type BlindedPubkeyType = `15${string}`;

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
    secretKey?: Uint8ArrayLen64; // len 64
  };

  export type GroupInfoSet = GroupInfoShared & {};

  export type SwarmSubAccountSignResult = {
    subaccount: string;
    subaccount_sig: string; // keeping the case like this so we can send it as is (this is what the snode API expects)
    signature: string;
  };

  // those types are not enforced currently, they all are just Uint8Arrays, but having them separate right away will make the enforcing of them later, easier
  export type Uint8ArrayLen64 = Uint8Array;
  export type Uint8ArrayLen32 = Uint8Array;
  export type Uint8ArrayLen100 = Uint8Array;
}
