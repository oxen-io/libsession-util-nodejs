/// <reference path="../shared.d.ts" />

declare module 'libsession_util_nodejs' {
  type MultiEncryptWrapper = {
    multiEncrypt: (opts: {
      /**
       * len 64: ed25519 secretKey with pubkey
       */
      ed25519SecretKey: Uint8ArrayLen64;
      domain: string;
      messages: Array<Uint8Array>;
      recipients: Array<Uint8Array>;
    }) => Uint8Array;
    multiDecryptEd25519: (opts: {
      encoded: Uint8Array;
      /**
       * len 64: ed25519 secretKey with pubkey
       */
      ed25519SecretKey: Uint8ArrayLen64;
      senderEd25519Pubkey: Uint8Array;
      domain: string;
    }) => Uint8Array | null;
  };

  export type MultiEncryptActionsCalls = MakeWrapperActionCalls<MultiEncryptWrapper>;

  /**
   * To be used inside the web worker only (calls are synchronous and won't work asynchrously)
   */
  export class MultiEncryptWrapperNode {
    public static multiEncrypt: MultiEncryptWrapper['multiEncrypt'];
    public static multiDecryptEd25519: MultiEncryptWrapper['multiDecryptEd25519'];
  }

  /**
   * Those actions are used internally for the web worker communication.
   * You should never need to import them in Session directly
   * You will need to add an entry here if you add a new function
   */
  export type MultiEncryptActionsType =
    | MakeActionCall<MultiEncryptWrapper, 'multiEncrypt'>
    | MakeActionCall<MultiEncryptWrapper, 'multiDecryptEd25519'>;
}
