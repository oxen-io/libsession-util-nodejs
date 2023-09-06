/// <reference path="../shared.d.ts" />

declare module 'libsession_util_nodejs' {
  export type GroupKeysWrapper = {
    keysNeedsRekey: () => boolean;
    keyRekey: () => Uint8Array;
    groupKeys: () => Array<Uint8Array>;

    loadKeyMessage: (hash: string, data: Uint8Array, timestampMs: number) => boolean;
    currentHashes: () => Array<string>;

    encryptMessage: (plaintext: Uint8Array, compress: boolean) => Uint8Array;
    decryptMessage: (ciphertext: Uint8Array) => Uint8Array | null;
  };
}
