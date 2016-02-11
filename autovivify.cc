#include <nan.h>
using namespace std;

#if NODE_MODULE_VERSION < IOJS_3_0_MODULE_VERSION
typedef v8::Handle<v8::Object> ADDON_REGISTER_FUNCTION_ARGS2_TYPE;
#else
typedef v8::Local<v8::Value> ADDON_REGISTER_FUNCTION_ARGS2_TYPE;
#endif

int ind = 0;

class AutoVivify : public Nan::ObjectWrap {
public:
  static void Init(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE exports, ADDON_REGISTER_FUNCTION_ARGS2_TYPE module);
  AutoVivify();
  int index() {return i;}

private:
  Nan::Persistent<v8::Object> backing_obj;
  int i;
  static NAN_METHOD(New);
  static NAN_METHOD(inspect);
  static NAN_PROPERTY_SETTER(PropSetter);
  static NAN_PROPERTY_GETTER(PropGetter);
  static NAN_PROPERTY_QUERY(PropQuery);
  static NAN_PROPERTY_DELETER(PropDeleter);
  static NAN_PROPERTY_ENUMERATOR(PropEnumerator);
  static NAN_INDEX_GETTER(IndexGetter);
  static NAN_INDEX_SETTER(IndexSetter);
  static NAN_INDEX_QUERY(IndexQuery);
  static NAN_INDEX_DELETER(IndexDeleter);
  static NAN_INDEX_ENUMERATOR(IndexEnumerator);
  
  static inline Nan::Persistent<v8::Function> & constructor() {
    static Nan::Persistent<v8::Function> my_constructor;
    return my_constructor;
  }
};

AutoVivify::AutoVivify() {
  i = ind++;
}

NAN_PROPERTY_SETTER(AutoVivify::PropSetter) {
  v8::String::Utf8Value prop(property);

  AutoVivify *self = Nan::ObjectWrap::Unwrap<AutoVivify>(info.This());
  v8::Local<v8::Object> object = Nan::New(self->backing_obj);

#if (NODE_MODULE_VERSION > NODE_0_10_MODULE_VERSION)
  if (property->IsSymbol()) {
    Nan::ThrowError("Symbol properties are not supported.");
    return;
  }
#endif
  if (!Nan::Set(object, property, value).FromMaybe(false)) {
    Nan::ThrowError("Error setting.");
    return;
  }
}

NAN_PROPERTY_GETTER(AutoVivify::PropGetter) {
  v8::String::Utf8Value data(info.Data());
  v8::String::Utf8Value src(property);
  if (string(*src) == "prototype") {
    info.GetReturnValue().Set(info.This()->GetPrototype());
    return;
  }
  
  if (string(*src) == "inspect") {
    v8::Local<v8::FunctionTemplate> tmpl = Nan::New<v8::FunctionTemplate>(inspect);
    v8::Local<v8::Function> fn = Nan::GetFunction(tmpl).ToLocalChecked();
    fn->SetName(Nan::New("inspect").ToLocalChecked());
    info.GetReturnValue().Set(fn);
    return;
  }
  
  AutoVivify *self = Nan::ObjectWrap::Unwrap<AutoVivify>(info.This());
  v8::Local<v8::Object> object = Nan::New(self->backing_obj);

  if (string(*src) == "length") {
    uint32_t length = object->Get(Nan::New("length").ToLocalChecked())->ToObject()->Uint32Value();
    info.GetReturnValue().Set(length);
    return;
  }
  if (string(*src) == "valueOf")
    return;
  if (string(*src) == "toString")
    return;
  if (string(*src) == "nodeType")
    return;

  v8::Local<v8::Value> val = Nan::Get(object, property).ToLocalChecked();
  if (val->IsUndefined()) {
    v8::Local<v8::Function> cons = Nan::New(constructor());
    v8::Local<v8::Value> newobj = cons->NewInstance();
    if (!Nan::Set(object, property, newobj).FromMaybe(false)) {
      Nan::ThrowError("Error setting.");
      return;
    }
    info.GetReturnValue().Set(newobj); // this.av->handle());
  } else {
    info.GetReturnValue().Set(val);
  }
}

NAN_PROPERTY_QUERY(AutoVivify::PropQuery) {
  v8::String::Utf8Value src(property);
  if (string(*src) == "prototype") {
    info.GetReturnValue().Set(Nan::New<v8::Integer>(v8::None));
  }
  AutoVivify *self = Nan::ObjectWrap::Unwrap<AutoVivify>(info.This());
  v8::Local<v8::Object> object = Nan::New(self->backing_obj);
  v8::Local<v8::Value> val = Nan::Get(object, property).ToLocalChecked();
  if (!val->IsUndefined()) {
    info.GetReturnValue().Set(Nan::New<v8::Integer>(v8::None));
  }
}

NAN_PROPERTY_DELETER(AutoVivify::PropDeleter) {
  AutoVivify *self = Nan::ObjectWrap::Unwrap<AutoVivify>(info.This());
  v8::Local<v8::Object> object = Nan::New(self->backing_obj);
  Nan::Delete(object, property);
  return;
}

NAN_PROPERTY_ENUMERATOR(AutoVivify::PropEnumerator) {
  AutoVivify *self = Nan::ObjectWrap::Unwrap<AutoVivify>(info.This());
  v8::Local<v8::Object> object = Nan::New(self->backing_obj);
  if (object->IsObject()) {
    info.GetReturnValue().Set(object->GetPropertyNames());
  }
}

NAN_INDEX_GETTER(AutoVivify::IndexGetter) {
  AutoVivify *self = Nan::ObjectWrap::Unwrap<AutoVivify>(info.This());
  v8::Local<v8::Object> object = Nan::New(self->backing_obj);

  if (object->IsObject()) { // Is it empty?
    v8::Local<v8::Array> props = object->GetPropertyNames();
    int length = props->Get(Nan::New("length").ToLocalChecked())->ToObject()->Uint32Value();
    if (length == 0) { // Replace it.
      v8::Local<v8::Array> new_array = Nan::New<v8::Array>();
      self->backing_obj.Reset(new_array);
      object = Nan::New(self->backing_obj);
    }
  } else { // Nope, treat as object.
    return;
  }

  v8::Local<v8::Value> val = Nan::Get(object, index).ToLocalChecked();
  if (val->IsUndefined()) {
    v8::Local<v8::Function> cons = Nan::New(constructor());
    v8::Local<v8::Object> newobj = cons->NewInstance();
    if (!Nan::Set(object, index, newobj).FromMaybe(false)) {
      Nan::ThrowError("Error setting.");
      return;
    }
    info.GetReturnValue().Set(newobj);
    return;
  }
  info.GetReturnValue().Set(val);
}

NAN_INDEX_SETTER(AutoVivify::IndexSetter) {
  AutoVivify *self = Nan::ObjectWrap::Unwrap<AutoVivify>(info.This());
  v8::Local<v8::Object> object = Nan::New(self->backing_obj);
  if (object->IsObject()) { // Is it empty?
    v8::Local<v8::Array> props = object->GetPropertyNames();
    int length = props->Get(Nan::New("length").ToLocalChecked())->ToObject()->Uint32Value();
    if (length == 0) { // Replace it.
      v8::Local<v8::Array> new_array = Nan::New<v8::Array>();
      self->backing_obj.Reset(new_array);
      object = Nan::New(self->backing_obj);
    }
  } else { // Nope, treat as object.
    return;
  }
  
  if (!object->IsArray())
    return;

  if (!Nan::Set(object, index, value).FromMaybe(false)) {
    Nan::ThrowError("Error setting array value");
    return;
  }
}

NAN_INDEX_QUERY(AutoVivify::IndexQuery) {
  v8::Local<v8::Integer> No;

  AutoVivify *self = Nan::ObjectWrap::Unwrap<AutoVivify>(info.This());
  v8::Local<v8::Object> object = Nan::New(self->backing_obj);
  if(!object->IsArray())
    return;
  v8::Local<v8::Value> val = Nan::Get(object, index).ToLocalChecked();
  if (val->IsUndefined()) {
    info.GetReturnValue().Set(No);
    return;
  }
  info.GetReturnValue().Set(Nan::New<v8::Integer>(v8::None));
}

NAN_INDEX_DELETER(AutoVivify::IndexDeleter) {
  AutoVivify *self = Nan::ObjectWrap::Unwrap<AutoVivify>(info.This());
  v8::Local<v8::Object> object = Nan::New(self->backing_obj);
  Nan::Delete(object, index);
  return;
}

NAN_INDEX_ENUMERATOR(AutoVivify::IndexEnumerator) {
  AutoVivify *self = Nan::ObjectWrap::Unwrap<AutoVivify>(info.This());
  v8::Local<v8::Object> object = Nan::New(self->backing_obj);
  if(!object->IsArray())
    return;
  int length = object->Get(Nan::New("length").ToLocalChecked())->ToObject()->Uint32Value();
  v8::Local<v8::Array> indexes = Nan::New<v8::Array>();

  for (int i = 0; i < length; ++i) {
    v8::Local<v8::Value> val = Nan::Get(object, i).ToLocalChecked();
    if (!val->IsUndefined())
      Nan::Set(indexes, i, Nan::New<v8::Number>(i));
  }

  info.GetReturnValue().Set(indexes);
}

NAN_METHOD(AutoVivify::New) {
  if (!info.IsConstructCall()) {
    Nan::ThrowError("Function must be called as a constructor.");
    return;
  }
  AutoVivify *av = new AutoVivify();
  av->backing_obj.Reset(Nan::New<v8::Object>());

  av->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(AutoVivify::inspect) {
  AutoVivify *self = Nan::ObjectWrap::Unwrap<AutoVivify>(info.This());
  v8::Local<v8::Object> object = Nan::New(self->backing_obj);

  info.GetReturnValue().Set(object);
}


void AutoVivify::Init(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE exports, ADDON_REGISTER_FUNCTION_ARGS2_TYPE module) {
    v8::Local<v8::FunctionTemplate> tmpl = Nan::New<v8::FunctionTemplate>(New);
    tmpl->SetClassName(Nan::New("AutoVivify").ToLocalChecked());

    tmpl->InstanceTemplate()->SetInternalFieldCount(1);

    v8::Local<v8::ObjectTemplate> proto = tmpl->PrototypeTemplate();
    Nan::SetNamedPropertyHandler(proto, PropGetter, PropSetter, PropQuery, PropDeleter, PropEnumerator,
                                 Nan::New<v8::String>("prototype").ToLocalChecked());

    proto->SetInternalFieldCount(1);
    v8::Local<v8::ObjectTemplate> inst = tmpl->InstanceTemplate();
    inst->SetInternalFieldCount(1);
    Nan::SetNamedPropertyHandler(inst, PropGetter, PropSetter, PropQuery, PropDeleter, PropEnumerator,
                                 Nan::New<v8::String>("instance").ToLocalChecked());
    Nan::SetIndexedPropertyHandler(inst, IndexGetter, IndexSetter, IndexQuery, IndexDeleter, IndexEnumerator,
                                   Nan::New<v8::String>("instance").ToLocalChecked());
    
    constructor().Reset(Nan::GetFunction(tmpl).ToLocalChecked());
    Nan::Set(module.As<v8::Object>(), Nan::New("exports").ToLocalChecked(),
             Nan::GetFunction(tmpl).ToLocalChecked());
}

NODE_MODULE(autovivify, AutoVivify::Init)
