#include "../quickjs/quickjs.h"
#include <stdio.h>

// ANSI 转义码颜色定义
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_RESET "\x1b[0m"

// 实现 console.log 的原生函数
static JSValue js_console_log(JSContext *ctx, JSValueConst this_val, int argc,
                              JSValueConst *argv) {
  for (int i = 0; i < argc; i++) {
    const char *str = JS_ToCString(ctx, argv[i]);
    if (!str) {
      // 处理转换失败的情况
      return JS_EXCEPTION;
    }
    printf("%s%s", i > 0 ? " " : "", str);
    JS_FreeCString(ctx, str);
  }
  printf("\n");
  return JS_UNDEFINED;
}

// 实现 console.warn 的原生函数

// 实现 console.warn 的原生函数
static JSValue js_console_warn(JSContext *ctx, JSValueConst this_val, int argc,
                               JSValueConst *argv) {

  fprintf(stderr, ANSI_COLOR_YELLOW "%s: ", "WARN");

  for (int i = 0; i < argc; i++) {
    const char *str = JS_ToCString(ctx, argv[i]);
    if (!str) {
      // 处理转换失败的情况
      return JS_EXCEPTION;
    }
    fprintf(stderr, "%s%s", i > 0 ? " " : "", str);
    JS_FreeCString(ctx, str);
  }
  fprintf(stderr, ANSI_COLOR_RESET "\n");
  return JS_UNDEFINED;
}

// 实现 console.error 的原生函数
static JSValue js_console_error(JSContext *ctx, JSValueConst this_val, int argc,
                                JSValueConst *argv) {

  fprintf(stderr, ANSI_COLOR_RED "%s: ", "ERROR");

  for (int i = 0; i < argc; i++) {
    const char *str = JS_ToCString(ctx, argv[i]);
    if (!str) {
      // 处理转换失败的情况
      return JS_EXCEPTION;
    }
    fprintf(stderr, "%s%s", i > 0 ? " " : "", str);
    JS_FreeCString(ctx, str);
  }
  fprintf(stderr, ANSI_COLOR_RESET "\n");

  return JS_UNDEFINED;
}

// 注册 console 对象和其方法
void js_std_init_console(JSContext *ctx) {
  JSValue global_obj = JS_GetGlobalObject(ctx);
  JSValue console = JS_NewObject(ctx);

  // 添加 log 方法
  JS_SetPropertyStr(ctx, console, "log",
                    JS_NewCFunction(ctx, js_console_log, "log", 1));

  // 添加 warn 方法
  JS_SetPropertyStr(ctx, console, "warn",
                    JS_NewCFunction(ctx, js_console_warn, "warn", 1));

  // 添加 error 方法
  JS_SetPropertyStr(ctx, console, "error",
                    JS_NewCFunction(ctx, js_console_error, "error", 1));

  // 将 console 对象挂载到全局对象
  JS_SetPropertyStr(ctx, global_obj, "console", console);

  JS_FreeValue(ctx, global_obj);
}