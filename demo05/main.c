#include "../helpers/console.c"
#include "../helpers/exception.c"
#include "../helpers/file.c"
#include "../quickjs/quickjs.h"
#include "./point.c"
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
  JSRuntime *rt = JS_NewRuntime();
  JSContext *ctx = JS_NewContext(rt);

  js_std_init_console(ctx);

  // Initialize Point module
  js_init_module(ctx, "point");

  char *js_code = read_file_to_string("point.js");

  JSValue val =
      JS_Eval(ctx, js_code, strlen(js_code), "a.js", JS_EVAL_TYPE_MODULE);
  free(js_code);

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