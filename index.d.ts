/// <reference path="./shared.d.ts" />
/// <reference path="./user/index.d.ts" />
/// <reference path="./types/index.d.ts" />

declare module 'libsession_util_nodejs' {
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
}
