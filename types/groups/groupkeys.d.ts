/// <reference path="../shared.d.ts" />

declare module 'libsession_util_nodejs' {
  export type GroupKeysWrapper = {
    // GroupKeys related methods
    keysNeedsRekey: () => boolean;
    keyRekey: () => Uint8Array;
    keyGetAll: () => Array<Uint8Array>;
    loadKeyMessage: (hash: string, data: Uint8Array, timestampMs: number) => boolean;

    currentHashes: () => Array<string>;
    encryptMessages: (plaintexts: Array<Uint8Array>) => Array<Uint8Array>;
    decryptMessage: (ciphertext: Uint8Array) => { pubkeyHex: string; plaintext: Uint8Array };
    makeSwarmSubAccount: (memberPubkeyHex: PubkeyType) => Uint8ArrayLen100;
    generateSupplementKeys: (membersPubkeyHex: Array<PubkeyType>) => Array<Uint8Array>;
    swarmSubaccountSign: (
      message: Uint8Array,
      authData: Uint8ArrayLen100
    ) => SwarmSubAccountSignResult;

    swarmSubAccountToken: (memberPk: PubkeyType) => string; // hex encoded
    swarmVerifySubAccount: (signingValue: Uint8ArrayLen100) => boolean;
    loadAdminKeys: (secret: Uint8ArrayLen64) => void;
  };
}
