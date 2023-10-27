/// <reference path="../shared.d.ts" />

declare module 'libsession_util_nodejs' {
  export type GroupKeysWrapper = {
    // GroupKeys related methods
    keysNeedsRekey: () => boolean;
    keyRekey: () => Uint8Array;
    loadKeyMessage: (hash: string, data: Uint8Array, timestampMs: number) => boolean;

    currentHashes: () => Array<string>;
    encryptMessage: (plaintext: Uint8Array) => Uint8Array;
    decryptMessage: (ciphertext: Uint8Array) => { pubkeyHex: string; plaintext: Uint8Array };
    makeSwarmSubAccount: (memberPubkeyHex: PubkeyType) => Uint8ArrayLen100;
    swarmSubaccountSign: (
      message: Uint8Array,
      authData: Uint8ArrayLen100
    ) => SwarmSubAccountSignResult;
  };
}
