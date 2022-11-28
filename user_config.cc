#include "user_config.h"


NAN_METHOD(aString) {
    info.GetReturnValue().Set(Nan::New("Hello from Session Util Wrapper.").ToLocalChecked());
}
