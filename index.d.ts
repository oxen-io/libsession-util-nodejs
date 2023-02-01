declare module 'session_util_wrapper' {

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

  /**
   *
   * Utilities
   *
   */

  type AsyncWrapper<T extends (...args: any) => any> = (
    ...args: Parameters<T>
  ) => Promise<ReturnType<T>>;



  type MakeWrapperActionCalls<Type extends BaseConfigWrapper> = {
    [Property in keyof Type]: AsyncWrapper<Type[Property]>
  };


  export type ProfilePicture = {
    url: string | null;
    key: Uint8Array | null;
  };

  export type PushConfigResult = { data: Uint8Array; seqno: number }


  type MakeActionCall<A extends BaseConfigWrapper, B extends string> = [
    B,
    ...Parameters<A[B]>
  ];

  /**
   *
   * Base Config wrapper logic
   *
   */

  type BaseConfigWrapper = {
      needsDump: ()=> boolean;
      needsPush: ()=> boolean;
      push: ()=> PushConfigResult;
      dump: ()=> Uint8Array;
      confirmPushed: (seqno: number) => void;
      merge: (toMerge: Array<Uint8Array>)=>  number;
      storageNamespace: ()=>  number;
  }

  type MakeActionsCall<A extends BaseConfigWrapper, B extends string> = [
    B,
    ...Parameters<A[B]>
  ];
  export type BaseConfigActions =
  | MakeActionCall<BaseConfigWrapper, 'needsDump'>
  | MakeActionCall<BaseConfigWrapper, 'needsPush'>
  | MakeActionCall<BaseConfigWrapper, 'push'>
  | MakeActionCall<BaseConfigWrapper, 'dump'>
  | MakeActionCall<BaseConfigWrapper, 'confirmPushed'>
  | MakeActionCall<BaseConfigWrapper, 'merge'>
  | MakeActionCall<BaseConfigWrapper, 'storageNamespace'>;

  export abstract class BaseConfigWrapperInsideWorker {
    public needsDump: BaseConfigWrapper['needsDump']
    public needsPush: BaseConfigWrapper['needsPush']
    public push: BaseConfigWrapper['push']
    public dump: BaseConfigWrapper['dump']
    public confirmPushed: BaseConfigWrapper['confirmPushed']
    public merge: BaseConfigWrapper['merge']
    public storageNamespace: BaseConfigWrapper['storageNamespace']
  }

  export type BaseWrapperActionsCalls = MakeWrapperActionCalls<BaseConfigWrapper>;

  /**
   *
   * User config wrapper logic
   *
   */


  type UserConfigWrapper = BaseConfigWrapper & {
    init: (secretKey: Uint8Array, dump: Uint8Array | null) => void;
    getName:()=> string;
    setName: (name: string)=> void;
    getProfilePicture: ()=> ProfilePicture;
    setProfilePicture: (url: string, key: Uint8Array) =>  void;
  }


  export type UserConfigWrapperActionsCalls = MakeWrapperActionCalls<UserConfigWrapper>;



  /**
   * To be used inside the web worker only (calls are synchronous and won't work asynchrously)
   */
  export class UserConfigWrapperInsideWorker extends BaseConfigWrapperInsideWorker {
    constructor(secretKey: Uint8Array, dump: Uint8Array | null);
    public getName: UserConfigWrapper['getName'];
    public setName: UserConfigWrapper['setName'];
    public getProfilePicture: UserConfigWrapper['getProfilePicture'];
    public setProfilePicture: UserConfigWrapper['setProfilePicture'];
  }



  /**
   * Those actions are used internally for the web worker communication.
   * You should never need to import them in Session directly
   * You will need to add an entry here if you add a new function
   */
  export type UserConfigActionsType =
    | ['init', Uint8Array, Uint8Array | null]
    | MakeActionCall<UserConfigWrapper, 'getName'>
    | MakeActionCall<UserConfigWrapper, 'setName'>
    | MakeActionCall<UserConfigWrapper, 'getProfilePicture'>
    | MakeActionCall<UserConfigWrapper, 'setProfilePicture'>;


  /**
   *
   * Contacts wrapper logic
   *
   */

  type ContactsWrapper = BaseConfigWrapper & {
    init: (secretKey: Uint8Array, dump: Uint8Array | null) => void;
    get:(pubkeyHex: string)=> ContactInfo | null;
    getOrCreate:(pubkeyHex: string)=>ContactInfo;
    set:(contact: ContactInfo)=> void;
    setName:(pubkeyHex: string, name: string)=> void;
    setNickname:(pubkeyHex: string, nickname: string)=> void;
    setApproved:(pubkeyHex: string, approved: boolean)=> void;
    setApprovedMe:(pubkeyHex: string, approvedMe: boolean) => void;
    setBlocked:(pubkeyHex: string, blocked: boolean)=> void;
    setProfilePicture: (pubkeyHex: string, url: string, key: Uint8Array) => void;
    getAll: () => Array<ContactInfo>;
    erase: (pubkeyHex: string)=> void;
  }


  export type ContactsWrapperActionsCalls = MakeWrapperActionCalls<ContactsWrapper>;



  export type ContactInfo = {
    id: string;
    name?: string;
    nickname?: string;
    profilePicture?: ProfilePicture;
    approved?: boolean;
    approvedMe?: boolean;
    blocked?: boolean;
  };

  export class ContactsConfigWrapperInsideWorker extends BaseConfigWrapperInsideWorker {
    constructor(secretKey: Uint8Array, dump: Uint8Array | null);
    public get: ContactsWrapper['get'];
    public getOrCreate: ContactsWrapper['getOrCreate'];
    public set: ContactsWrapper['set'];
    public setName: ContactsWrapper['setName'];
    public setNickname: ContactsWrapper['setNickname'];
    public setApproved: ContactsWrapper['setApproved'];
    public setApprovedMe: ContactsWrapper['setApprovedMe'];
    public setBlocked: ContactsWrapper['setBlocked'];
    public setProfilePicture: ContactsWrapper['setProfilePicture'];
    public getAll: ContactsWrapper['getAll'];
    public erase: ContactsWrapper['erase'];
  }

  export type ContactsConfigActionsType =
    | ['init', Uint8Array, Uint8Array | null]
    | MakeActionCall<ContactsWrapper, 'get'>
    | MakeActionCall<ContactsWrapper, 'getOrCreate'>
    | MakeActionCall<ContactsWrapper, 'set'>
    | MakeActionCall<ContactsWrapper, 'setName'>
    | MakeActionCall<ContactsWrapper, 'setNickname'>
    | MakeActionCall<ContactsWrapper, 'setApproved'>
    | MakeActionCall<ContactsWrapper, 'setApprovedMe'>
    | MakeActionCall<ContactsWrapper, 'setBlocked'>
    | MakeActionCall<ContactsWrapper, 'setProfilePicture'>
    | MakeActionCall<ContactsWrapper, 'getAll'>
    | MakeActionCall<ContactsWrapper, 'erase'>;
}
