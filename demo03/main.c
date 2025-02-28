#include "../quickjs/quickjs.h"
#include <stdio.h>
#include <string.h>

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

int main(int argc, char **argv) {
  JSRuntime *rt = JS_NewRuntime();
  JSContext *ctx = JS_NewContext(rt);

  // Get the global object
  JSValue global_obj = JS_GetGlobalObject(ctx);
  JSValue console = JS_NewObject(ctx);

  JS_SetPropertyStr(ctx, console, "log",
                    JS_NewCFunction(ctx, js_print, "log", 1));
  JS_SetPropertyStr(ctx, global_obj, "console", console);

  // Execute JavaScript code that uses the C function
  const char *user_code = "console.log('Hello World!')";

  JSValue val = JS_Eval(ctx, user_code, strlen(user_code), "<input>",
                        JS_EVAL_TYPE_GLOBAL);

  if (JS_IsException(val)) {
    JSValue exc = JS_GetException(ctx);
    const char *str = JS_ToCString(ctx, exc);
    fprintf(stderr, "Error: %s\n", str);
    JS_FreeCString(ctx, str);
    JS_FreeValue(ctx, exc);

    return 1;
  }

  JS_FreeValue(ctx, val);
  JS_FreeValue(ctx, global_obj);
  JS_FreeContext(ctx);
  JS_FreeRuntime(rt);

  return 0;
}