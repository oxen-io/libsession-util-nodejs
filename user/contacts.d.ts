/**
 *
 * Contacts wrapper logic
 *
 */
declare module 'libsession_util_nodejs' {
  type ContactsWrapper = BaseConfigWrapper & {
    init: (secretKey: Uint8Array, dump: Uint8Array | null) => void;
    get: (pubkeyHex: string) => ContactInfo | null;
    set: (contact: ContactInfoSet) => void;
    getAll: () => Array<ContactInfo>;
    erase: (pubkeyHex: string) => void;
  };

  export type ContactsWrapperActionsCalls = MakeWrapperActionCalls<ContactsWrapper>;

  type ContactInfoShared = PriorityType & {
    id: string;
    name?: string;
    nickname?: string;
    profilePicture?: ProfilePicture;
    createdAtSeconds: number; // can only be set the first time a contact is created, a new change won't overide the value in the wrapper.

    // expirationMode: 'off' | 'disappearAfterRead' | 'disappearAfterSend'; // the same as defined in the disappearingBranch
    // expirationTimerSeconds: number;
  };

  export type ContactInfoSet = ContactInfoShared & {
    approved?: boolean;
    approvedMe?: boolean;
    blocked?: boolean;
  };

  export type ContactInfo = ContactInfoShared & {
    approved: boolean;
    approvedMe: boolean;
    blocked: boolean;
  };

  export class ContactsConfigWrapperNode extends BaseConfigWrapperNode {
    constructor(secretKey: Uint8Array, dump: Uint8Array | null);
    public get: ContactsWrapper['get'];
    public set: ContactsWrapper['set'];
    public getAll: ContactsWrapper['getAll'];
    public erase: ContactsWrapper['erase'];
  }

  export type ContactsConfigActionsType =
    | ['init', Uint8Array, Uint8Array | null]
    | MakeActionCall<ContactsWrapper, 'get'>
    | MakeActionCall<ContactsWrapper, 'set'>
    | MakeActionCall<ContactsWrapper, 'getAll'>
    | MakeActionCall<ContactsWrapper, 'erase'>;
}
