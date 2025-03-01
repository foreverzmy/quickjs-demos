#include "../helpers/exception.c"
#include "../quickjs/quickjs.h"
#include <stdio.h>
#include <string.h>

// C function to be called from JavaScript
static JSValue js_add(JSContext *ctx, JSValueConst this_val, int argc,
                      JSValueConst *argv) {
  int a, b;
  if (JS_ToInt32(ctx, &a, argv[0]))
    return JS_EXCEPTION;
  if (JS_ToInt32(ctx, &b, argv[1]))
    return JS_EXCEPTION;
  return JS_NewInt32(ctx, a + b);
}

int main(int argc, char **argv) {
  JSRuntime *rt = JS_NewRuntime();
  JSContext *ctx = JS_NewContext(rt);

  // Get the global object
  JSValue global_obj = JS_GetGlobalObject(ctx);

  // Define the C function in JavaScript environment
  JSValue add_func = JS_NewCFunction(ctx, js_add, "add", 2);
  JS_SetPropertyStr(ctx, global_obj, "add", add_func);

  // Execute JavaScript code that uses the C function
  const char *user_code = "add(11, 22)";

  JSValue val = JS_Eval(ctx, user_code, strlen(user_code), "<input>",
                        JS_EVAL_TYPE_GLOBAL);

  if (JS_IsException(val)) {
    check_and_print_exception(ctx);

    return 1;
  }

  int result;
  if (JS_ToInt32(ctx, &result, val))
    return 1;
  printf("Result: %d\n", result);

  JS_FreeValue(ctx, val);
  JS_FreeValue(ctx, global_obj);

  JS_RunGC(rt);

  JS_FreeContext(ctx);
  JS_FreeRuntime(rt);

  return 0;
}
