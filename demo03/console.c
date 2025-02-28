#include "../quickjs/quickjs.h"
#include <stdio.h>

static JSValue js_print(JSContext *ctx, JSValueConst this_val, int argc,
                        JSValueConst *argv) {
  int i;
  const char *str;
  size_t len;

  for (i = 0; i < argc; i++) {
    if (i != 0)
      putchar(' ');
    str = JS_ToCStringLen(ctx, &len, argv[i]);
    if (!str)
      return JS_EXCEPTION;
    fwrite(str, 1, len, stdout);
    JS_FreeCString(ctx, str);
  }
  putchar('\n');
  return JS_UNDEFINED;
}

static void init_std_console(JSContext *ctx) {
  JSValue global_obj = JS_GetGlobalObject(ctx);
  JSValue console = JS_NewObject(ctx);

  JS_SetPropertyStr(ctx, console, "log",
                    JS_NewCFunction(ctx, js_print, "log", 1));
  JS_SetPropertyStr(ctx, global_obj, "console", console);

  JS_FreeValue(ctx, global_obj);
}
