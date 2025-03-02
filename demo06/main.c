#include "../helpers/console.c"
#include "../helpers/exception.c"
#include "../quickjs/quickjs.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
  JSRuntime *rt = JS_NewRuntime();
  JSContext *ctx = JS_NewContext(rt);

  JS_SetMemoryLimit(rt, 1024 * 1024 * 1024); // 1 GB memory limit
  JS_SetMaxStackSize(rt, 1024 * 1024 * 100); // 100 MB stack size

  js_std_init_console(ctx);

  // Your JavaScript code can now use the Car class
  const char *js_code = "'use strict';"
                        "console.log('Hello World!');\n";

  JSValue fn_code = JS_Eval(ctx, js_code, strlen(js_code), "<input>",
                            JS_EVAL_FLAG_COMPILE_ONLY);

  if (JS_IsException(fn_code)) {
    check_and_print_exception(ctx);
    JS_FreeValue(ctx, fn_code);
    JS_FreeContext(ctx);
    JS_FreeRuntime(rt);
    return 1;
  }

  size_t bytecode_len;
  uint8_t *bytecode =
      JS_WriteObject(ctx, &bytecode_len, fn_code, JS_WRITE_OBJ_BYTECODE);
  JS_FreeValue(ctx, fn_code);
  if (!bytecode) {
    fprintf(stderr, "Failed to write bytecode\n");
    return 1;
  }

  JSValue fn = JS_ReadObject(ctx, bytecode, bytecode_len, JS_READ_OBJ_BYTECODE);
  js_free(ctx, bytecode);
  if (JS_IsException(fn)) {
    check_and_print_exception(ctx);
    return 1;
  }

  JSValue result = JS_EvalFunction(ctx, fn);

  // JSValue result = JS_Call(ctx, fn, JS_UNDEFINED, 0, NULL);
  if (JS_IsException(result)) {
    check_and_print_exception(ctx);
    JS_FreeValue(ctx, result);
    return 1;
  }

  JS_FreeValue(ctx, result);

  JS_RunGC(rt);

  JS_FreeContext(ctx);
  JS_FreeRuntime(rt);

  return 0;
}
