#include <nan.h>
using namespace std;

#if NODE_MODULE_VERSION < IOJS_3_0_MODULE_VERSION
typedef v8::Handle<v8::Object> ADDON_REGISTER_FUNCTION_ARGS2_TYPE;
#else
typedef v8::Local<v8::Value> ADDON_REGISTER_FUNCTION_ARGS2_TYPE;
#endif

class AutoVivify : public Nan::ObjectWrap {
public:
  static void Init(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE exports, ADDON_REGISTER_FUNCTION_ARGS2_TYPE module);

private:
  Nan::Persistent<v8::Object> backing_obj;
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
  template<typename T>
  static void Vivify(v8::Local<v8::Object> &,
                     T,
                     const Nan::PropertyCallbackInfo<v8::Value>*);
  static bool EnsureArray(v8::Local<v8::Object>&, AutoVivify *);
  static inline Nan::Persistent<v8::Function> & constructor() {
    static Nan::Persistent<v8::Function> my_constructor;
    return my_constructor;
  }
};

#define UNWRAPOBJECT                                                    \
  AutoVivify *self = Nan::ObjectWrap::Unwrap<AutoVivify>(info.This());  \
  v8::Local<v8::Object> object = Nan::New(self->backing_obj)

#define LENGTH(object)                                                  \
  object->Get(Nan::New("length").ToLocalChecked())->ToObject()->Uint32Value()

NAN_PROPERTY_SETTER(AutoVivify::PropSetter) {
  v8::String::Utf8Value prop(property);

  UNWRAPOBJECT;

#if (NODE_MODULE_VERSION > NODE_0_10_MODULE_VERSION)
  if (property->IsSymbol()) {
    Nan::ThrowError("Symbol properties are not supported.");
    return;
  }
#endif

  if (!Nan::Set(object, property, value).FromMaybe(false))
    Nan::ThrowError("Error setting property.");
}

template<typename T>
void AutoVivify::Vivify(v8::Local<v8::Object> &object,
                        T property_or_index,
                        const Nan::PropertyCallbackInfo<v8::Value> *info) {
  v8::Local<v8::Value> val = Nan::Get(object, property_or_index).ToLocalChecked();
  if (!val->IsUndefined()) {
    info->GetReturnValue().Set(val);
    return;
  }

  v8::Local<v8::Function> cons = Nan::New(constructor());
  v8::Local<v8::Value> newobj = cons->NewInstance();
  if (!Nan::Set(object, property_or_index, newobj).FromMaybe(false))
    Nan::ThrowError("Error autovivifying property.");
  else
    info->GetReturnValue().Set(newobj);
}

NAN_PROPERTY_GETTER(AutoVivify::PropGetter) {
  v8::String::Utf8Value data(info.Data());
  v8::String::Utf8Value src(property);
  if (string(*src) == "prototype" ||
      string(*src) == "valueOf" ||
      string(*src) == "toString" ||
      string(*src) == "nodeType")
    return;

  if (string(*src) == "inspect") {
    v8::Local<v8::FunctionTemplate> tmpl = Nan::New<v8::FunctionTemplate>(inspect);
    v8::Local<v8::Function> fn = Nan::GetFunction(tmpl).ToLocalChecked();
    fn->SetName(Nan::New("inspect").ToLocalChecked());
    info.GetReturnValue().Set(fn);
    return;
  }

  UNWRAPOBJECT;

  if (string(*src) == "length")
    info.GetReturnValue().Set(LENGTH(object));
  else
    Vivify(object, property, &info);
}

NAN_PROPERTY_QUERY(AutoVivify::PropQuery) {
  v8::String::Utf8Value src(property);
  if (string(*src) == "prototype") {
    info.GetReturnValue().Set(Nan::New<v8::Integer>(v8::None));
  }

  UNWRAPOBJECT;

  v8::Local<v8::Value> val = Nan::Get(object, property).ToLocalChecked();
  if (!val->IsUndefined())
    info.GetReturnValue().Set(Nan::New<v8::Integer>(v8::None));
}

NAN_PROPERTY_DELETER(AutoVivify::PropDeleter) {
  UNWRAPOBJECT;
  Nan::Delete(object, property);
}

NAN_PROPERTY_ENUMERATOR(AutoVivify::PropEnumerator) {
  UNWRAPOBJECT;
  if (object->IsObject())
    info.GetReturnValue().Set(object->GetPropertyNames());
}

bool AutoVivify::EnsureArray(v8::Local<v8::Object> &object, AutoVivify *self) {
  if (object->IsObject() && !object->IsArray()) {
    // Is it empty?
    v8::Local<v8::Array> props = object->GetPropertyNames();
    if (LENGTH(props) != 0) // No, bail.
      return false;
    // Replace with array
    v8::Local<v8::Array> new_array = Nan::New<v8::Array>();
    self->backing_obj.Reset(new_array);
    object = Nan::New(self->backing_obj);
  }
  return object->IsArray();
}

NAN_INDEX_GETTER(AutoVivify::IndexGetter) {
  UNWRAPOBJECT;

  if (EnsureArray(object, self))
    Vivify(object, index, &info);
}

NAN_INDEX_SETTER(AutoVivify::IndexSetter) {
  UNWRAPOBJECT;

  if (EnsureArray(object, self) &&
      !Nan::Set(object, index, value).FromMaybe(false))
    Nan::ThrowError("Error setting array value");
}

NAN_INDEX_QUERY(AutoVivify::IndexQuery) {
  UNWRAPOBJECT;

  if(object->IsArray() &&
     !Nan::Get(object, index).ToLocalChecked()->IsUndefined())
    info.GetReturnValue().Set(Nan::New<v8::Integer>(v8::None));
}

NAN_INDEX_DELETER(AutoVivify::IndexDeleter) {
  UNWRAPOBJECT;
  Nan::Delete(object, index);
}

NAN_INDEX_ENUMERATOR(AutoVivify::IndexEnumerator) {
  UNWRAPOBJECT;
  if(!object->IsArray())
    return;

  v8::Local<v8::Array> indexes = Nan::New<v8::Array>();

  int length = LENGTH(object);
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
  UNWRAPOBJECT;
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
