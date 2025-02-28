#include "../quickjs/quickjs.h"
#include <stdio.h>

// 定义一个宏方法，用于计算数组元素个数
#define countof(x) (sizeof(x) / sizeof((x)[0]))

// Vendor structure
typedef struct {
  char name;
} Vendor;

// Class ID，用于标识 Vendor 类
static JSClassID js_vendor_class_id;

// Finalizer to clean up, 定义析构函数，用于清理 Vendor 实例
static void js_vendor_finalizer(JSRuntime *rt, JSValue val) {
  Vendor *v = JS_GetOpaque(val, js_vendor_class_id);
  printf("===Vendor finalizer called===\n");
  js_free_rt(rt, v);
}

// Constructor，用于创建新的 Vendor 实例
static JSValue js_vendor_constructor(JSContext *ctx, JSValueConst new_target,
                                     int argc, JSValueConst *argv) {
  Vendor *v; // 声明 Vendor 结构体指针
  JSValue obj = JS_UNDEFINED;
  JSValue proto;
  const char *name;

  // Allocate the Vendor structure，为 Vendor 结构体分配内存并初始化为 0
  v = js_mallocz(ctx, sizeof(Vendor));
  if (!v) { // 检查内存分配是否成功
    return JS_EXCEPTION;
  }

  name = JS_ToCString(ctx, argv[0]);

  // Set the properties
  v->name = *name;

  // 释放临时字符串内存
  JS_FreeCString(ctx, name);

  // 获取构造函数的原型对象
  proto = JS_GetPropertyStr(ctx, new_target, "prototype");
  if (JS_IsException(proto))
    goto fail;

  // 创建新的 JS 对象，使用指定的原型和类 ID
  obj = JS_NewObjectProtoClass(ctx, proto, js_vendor_class_id);
  JS_FreeValue(ctx, proto);
  if (JS_IsException(obj))
    goto fail;

  // 将 Vendor 结构体绑定到 JS 对象
  JS_SetOpaque(obj, v);

  return obj;
fail:
  js_free(ctx, v);
  JS_FreeValue(ctx, obj);
  return JS_EXCEPTION;
}

// Age calculation method
static JSValue js_vendor_echo(JSContext *ctx, JSValueConst this_val, int argc,
                              JSValueConst *argv) {
  // 从 JS 对象中获取 Vendor 结构体指针
  Vendor *v = JS_GetOpaque2(ctx, this_val, js_vendor_class_id);
  // 检查是否成功获取 Vendor 结构体
  if (!v)
    return JS_EXCEPTION;

  printf("====C echo called===\n");
  return JS_NewString(ctx, &v->name);
}

// Class definition，定义 Vendor 类的基本信息
static JSClassDef js_vendor_class = {
    "Vendor",
    .finalizer = js_vendor_finalizer,
};

// Vendor class definition, 定义 Vendor 类的原型方法列表
static const JSCFunctionListEntry js_vendor_proto_funcs[] = {
    JS_CFUNC_DEF("echo", 0, js_vendor_echo),
};

// Initialize the class
static int js_vendor_init(JSContext *ctx) {
  JSValue proto, vendor_class;

  // Initialize the class ID
  JS_NewClassID(&js_vendor_class_id);
  // 在运行时中注册 Vendor 类
  JS_NewClass(JS_GetRuntime(ctx), js_vendor_class_id, &js_vendor_class);

  // Create prototype,创建类的原型对象
  proto = JS_NewObject(ctx);
  // 为原型对象添加方法
  JS_SetPropertyFunctionList(ctx, proto, js_vendor_proto_funcs,
                             countof(js_vendor_proto_funcs));

  // Create constructor，创建构造函数
  vendor_class = JS_NewCFunction2(ctx, js_vendor_constructor, "Vendor", 1,
                                  JS_CFUNC_constructor, 0);

  // 设置构造函数的原型
  JS_SetConstructor(ctx, vendor_class, proto);
  // 设置类的原型
  JS_SetClassProto(ctx, js_vendor_class_id, proto);

  JSValue global_obj = JS_GetGlobalObject(ctx);

  // Set the class as a global property
  JS_SetPropertyStr(ctx, global_obj, "Vendor", vendor_class);

  JS_FreeValue(ctx, global_obj);

  return 0;
}

void js_init_vendor_class(JSContext *ctx) { js_vendor_init(ctx); }
