declare module "session_util_wrapper" {
  export class UserConfigWrapper {
    constructor();
    constructor(restoreFromData: string);
    public getValue(): number;
  }
}
