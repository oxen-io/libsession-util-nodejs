declare module 'libsession_util_nodejs' {
  /**
   *
   * Utilities
   *
   */

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
}
