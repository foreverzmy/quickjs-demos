#include "../quickjs/quickjs.h"
#include <stdio.h>

// 定义一个宏方法，用于计算数组元素个数
#define countof(x) (sizeof(x) / sizeof((x)[0]))

// Class ID，用于标识 Car 类
static JSClassID js_car_class_id;

// Car structure
typedef struct {
  char dummy;
  // char brand[50]; // 存储汽车品牌，最大长度为 49 个字符
  // int year;       // 存储汽车生产年份
} Car;

// Constructor，用于创建新的 Car 实例
static JSValue js_car_constructor(JSContext *ctx, JSValueConst new_target,
                                  int argc, JSValueConst *argv) {
  Car *car; // 声明 Car 结构体指针
  JSValue obj = JS_UNDEFINED;
  JSValue proto;
  // const char *brand; // 存储品牌字符串
  // int year;          // 存储年份

  // Allocate the car structure，为 Car 结构体分配内存并初始化为 0
  car = js_mallocz(ctx, sizeof(Car));
  if (!car) { // 检查内存分配是否成功
    return JS_EXCEPTION;
  }

  // Get constructor parameters
  // brand = JS_ToCString(ctx, argv[0]);
  // JS_ToInt32(ctx, &year, argv[1]);

  // // Set the properties，将品牌名称复制到 Car 结构体中，确保不会缓冲区溢出
  // strncpy(car->brand, brand, sizeof(car->brand) - 1);
  // car->year = year;

  // // 释放临时字符串内存
  // JS_FreeCString(ctx, brand);

  // 获取构造函数的原型对象
  proto = JS_GetPropertyStr(ctx, new_target, "prototype");
  // 检查是否获取原型对象成功
  if (JS_IsException(proto))
    goto fail;

  // 创建新的 JS 对象，使用指定的原型和类 ID
  obj = JS_NewObjectProtoClass(ctx, proto, js_car_class_id);
  JS_FreeValue(ctx, proto);
  // 检查对象创建是否成功
  if (JS_IsException(obj))
    goto fail;

  // 将 Car 结构体绑定到 JS 对象
  JS_SetOpaque(obj, car);

  return obj;
fail:
  js_free(ctx, car);
  if (!JS_IsUndefined(proto))
    JS_FreeValue(ctx, proto);
  if (!JS_IsUndefined(obj))
    JS_FreeValue(ctx, obj);
  return JS_EXCEPTION;
}

// Age calculation method
static JSValue js_car_age(JSContext *ctx, JSValueConst this_val, int argc,
                          JSValueConst *argv) {
  // 从 JS 对象中获取 Car 结构体指针
  Car *car = JS_GetOpaque2(ctx, this_val, js_car_class_id);
  // int current_year;

  // 检查是否成功获取 Car 结构体
  if (!car)
    return JS_EXCEPTION;

  // 从参数中获取当前年份
  // JS_ToInt32(ctx, &current_year, argv[0]);
  // 计算并返回汽车年龄
  return JS_NewString(ctx, "foreverz");
}

// Car class definition, 定义 Car 类的原型方法列表
static const JSCFunctionListEntry js_car_proto_funcs[] = {
    JS_CFUNC_DEF("age", 0, js_car_age),
};

// Finalizer to clean up, 定义析构函数，用于清理 Car 实例
static void js_car_finalizer(JSRuntime *rt, JSValue val) {
  printf("Finalizer: freeing car\n");
  Car *car = JS_GetOpaque(val, js_car_class_id);
  if (car) {
    js_free_rt(rt, car);
  }
}

// Class definition，定义 Car 类的基本信息
static const JSClassDef js_car_class = {"Car", .finalizer = js_car_finalizer};

// Initialize the class
int js_car_init(JSContext *ctx) { // Remove 'static' here
  JSValue proto, obj;

  // Initialize the class ID
  JS_NewClassID(&js_car_class_id);
  // 在运行时中注册 Car 类
  JS_NewClass(JS_GetRuntime(ctx), js_car_class_id, &js_car_class);

  // Create prototype,创建类的原型对象
  proto = JS_NewObject(ctx);
  // 为原型对象添加方法
  JS_SetPropertyFunctionList(ctx, proto, js_car_proto_funcs,
                             countof(js_car_proto_funcs));

  // Create constructor，创建构造函数
  obj = JS_NewCFunction2(ctx, js_car_constructor, "Car", 1,
                         JS_CFUNC_constructor, 0);
  // 设置构造函数的原型
  JS_SetConstructor(ctx, obj, proto);
  // 设置类的原型
  JS_SetClassProto(ctx, js_car_class_id, proto);

  // Set the class as a global property
  JS_SetPropertyStr(ctx, JS_GetGlobalObject(ctx), "Car", obj);

  JS_FreeValue(ctx, obj);
  JS_FreeValue(ctx, proto);

  return 0;
}
