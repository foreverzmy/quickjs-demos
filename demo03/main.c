#include "../helpers/exception.c"
#include "../quickjs/quickjs.h"
#include "./console.c"
#include <string.h>

int main(int argc, char **argv) {
  JSRuntime *rt = JS_NewRuntime();
  JSContext *ctx = JS_NewContext(rt);

  js_std_init_console(ctx);

  // Execute JavaScript code that uses the C function
  const char *js_code = "console.log('Hello World!');"
                        "console.warn('Hello World!');"
                        "console.error('Hello World!');";

  JSValue val =
      JS_Eval(ctx, js_code, strlen(js_code), "<input>", JS_EVAL_TYPE_GLOBAL);

  if (JS_IsException(val)) {
    check_and_print_exception(ctx);
    return 1;
  }

  JS_FreeValue(ctx, val);

  JS_RunGC(rt);

  JS_FreeContext(ctx);
  JS_FreeRuntime(rt);

  return 0;
}