declare module 'session_util_wrapper' {
  export type ProfilePicture = {
    url: string | null;
    key: Uint8Array | null;
  };

  export abstract class BaseConfigWrapper {
    public needsDump(): boolean;
    public needsPush(): boolean;
    public push(): { data: Uint8Array; seqno: number };
    public dump(): Uint8Array;
    public confirmPushed(seqno: number): void;
    public merge(toMerge: Array<Uint8Array>): number;
    public storageNamespace(): number;
    public encryptionDomain(): string;
  }

  type MakeActionCall<A extends BaseConfigWrapper, B extends string> = [
    B,
    ...Parameters<A[B]>
  ]; // ReturnType<A[B]>,

  export type BaseConfigActions =
    | MakeActionCall<BaseConfigWrapper, 'needsDump'>
    | MakeActionCall<BaseConfigWrapper, 'needsPush'>
    | MakeActionCall<BaseConfigWrapper, 'push'>
    | MakeActionCall<BaseConfigWrapper, 'dump'>
    | MakeActionCall<BaseConfigWrapper, 'confirmPushed'>
    | MakeActionCall<BaseConfigWrapper, 'merge'>
    | MakeActionCall<BaseConfigWrapper, 'storageNamespace'>
    | MakeActionCall<BaseConfigWrapper, 'encryptionDomain'>;

  export class UserConfigWrapper extends BaseConfigWrapper {
    constructor(secretKey: Uint8Array, dump: Uint8Array | null);
    public getName(): string;
    public setName(name: string);
    public getProfilePicture(): ProfilePicture;
    public setProfilePicture(url: string | null, key: Uint8Array | null);
  }

  export type UserConfigActionsType =
    | ['init', Uint8Array, Uint8Array | null]
    | MakeActionCall<UserConfigWrapper, 'getName'>
    | MakeActionCall<UserConfigWrapper, 'setName'>
    | MakeActionCall<UserConfigWrapper, 'getProfilePicture'>
    | MakeActionCall<UserConfigWrapper, 'setProfilePicture'>;

  export type ContactInfo = {
    id: string;
    name?: string;
    nickname?: string;
    profilePicture?: ProfilePicture;
    approved?: boolean;
    approvedMe?: boolean;
    blocked?: boolean;
  };

  export class ContactsConfigWrapper extends BaseConfigWrapper {
    constructor(secretKey: Uint8Array, dump: Uint8Array | null);

    public get(pubkeyHex: string): ContactInfo | null;
    public getOrCreate(pubkeyHex: string): ContactInfo;
    public set(contact: ContactInfo);

    public setName(pubkeyHex: string, name: string);
    public setNickname(pubkeyHex: string, nickname: string);
    public setApproved(pubkeyHex: string, approved: boolean);
    public setApprovedMe(pubkeyHex: string, approvedMe: boolean);
    public setBlocked(pubkeyHex: string, blocked: boolean);
    public setProfilePicture(
      pubkeyHex: string,
      url: string | null,
      key: Uint8Array | null
    );

    public getAll(): Array<ContactInfo>;

    public erase(pubkeyHex: string);
  }
}
