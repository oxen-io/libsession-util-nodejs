declare module "session_util_wrapper" {
  export abstract class BaseConfigWrapper {
    public needsDump(): boolean;
    public needsPush(): boolean;
  }
  export class UserConfigWrapper extends BaseConfigWrapper {
    constructor();
    public getName(): string;
    public setName(name: string);
    public getProfilePic(): {
      url: string | null;
      key: Uint8Array | null;
    } | null;
    public setProfilePic(url: string | null, key: unknown | null);
  }
}
