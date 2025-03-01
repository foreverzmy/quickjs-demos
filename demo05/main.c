#include "../demo03/console.c"
#include "../helpers/file.c"
#include "../quickjs/quickjs.h"
#include "./point.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
  JSRuntime *rt = JS_NewRuntime();
  JSContext *ctx = JS_NewContext(rt);

  init_std_console(ctx);

  // Initialize Point module
  js_init_module(ctx, "point");

  char *js_code = read_file_to_string("point.js");

  JSValue val =
      JS_Eval(ctx, js_code, strlen(js_code), "a.js", JS_EVAL_TYPE_MODULE);
  free(js_code);

  if (JS_IsException(val)) {
    JSValue exc = JS_GetException(ctx);
    const char *str = JS_ToCString(ctx, exc);
    fprintf(stderr, "Error: %s\n", str);
    JS_FreeCString(ctx, str);
    JS_FreeValue(ctx, exc);
    JS_FreeValue(ctx, val);
    return 1;
  }

  JS_FreeValue(ctx, val);
  JS_FreeContext(ctx);
  JS_FreeRuntime(rt);
  return 0;
}