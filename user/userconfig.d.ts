declare module 'libsession_util_nodejs' {
  /**
   *
   * User config wrapper logic
   *
   */

  type UserConfigWrapper = BaseConfigWrapper & {
    init: (secretKey: Uint8Array, dump: Uint8Array | null) => void;
    /** This function is used to free wrappers from memory only */
    free: () => void;
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
    setNoteToSelfExpiry: (expirySeconds: number) => void;
    /**
     * @returns the expiry in seconds, 0 if off, undefined if not set
     */
    getNoteToSelfExpiry: () => number | undefined;
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
    public getNoteToSelfExpiry: UserConfigWrapper['getNoteToSelfExpiry'];
    public setNoteToSelfExpiry: UserConfigWrapper['setNoteToSelfExpiry'];
  }

  /**
   * Those actions are used internally for the web worker communication.
   * You should never need to import them in Session directly
   * You will need to add an entry here if you add a new function
   */
  export type UserConfigActionsType =
    | ['init', Uint8Array, Uint8Array | null]
    | MakeActionCall<UserConfigWrapper, 'free'>
    | MakeActionCall<UserConfigWrapper, 'getUserInfo'>
    | MakeActionCall<UserConfigWrapper, 'setUserInfo'>
    | MakeActionCall<UserConfigWrapper, 'getEnableBlindedMsgRequest'>
    | MakeActionCall<UserConfigWrapper, 'setEnableBlindedMsgRequest'>
    | MakeActionCall<UserConfigWrapper, 'getNoteToSelfExpiry'>
    | MakeActionCall<UserConfigWrapper, 'setNoteToSelfExpiry'>;
}
