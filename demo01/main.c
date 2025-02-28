#include "../quickjs/quickjs.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
  JSRuntime *rt = JS_NewRuntime();
  JSContext *ctx = JS_NewContext(rt);

  JSValue global_obj = JS_GetGlobalObject(ctx);

  // 执行用户代码
  const char *user_code = "function add(a, b) { return a + b; }";

  JSValue val = JS_Eval(ctx, user_code, strlen(user_code), "<input>",
                        JS_EVAL_TYPE_GLOBAL);

  if (JS_IsException(val)) {
    JSValue exc = JS_GetException(ctx);
    const char *str = JS_ToCString(ctx, exc);
    fprintf(stderr, "Error: %s\n", str);
    JS_FreeCString(ctx, str);
    JS_FreeValue(ctx, exc);
  }

  // Get the add function from the global object
  JSValue add_func = JS_GetPropertyStr(ctx, global_obj, "add");

  // Check if the add function is valid
  if (!JS_IsFunction(ctx, add_func)) {
    fprintf(stderr, "Error: add function not found\n");
    JS_FreeValue(ctx, add_func);
    JS_FreeValue(ctx, global_obj);
    JS_FreeContext(ctx);
    JS_FreeRuntime(rt);
    return 1;
  }

  // Call the add function with two integer arguments
  JSValue args[2];
  args[0] = JS_NewInt32(ctx, 11);
  args[1] = JS_NewInt32(ctx, 22);
  JSValue result = JS_Call(ctx, add_func, JS_UNDEFINED, 2, args);
  // Check if the result is valid
  if (!JS_IsNumber(result)) {
    fprintf(stderr, "Error: add function returned invalid result\n");
    JS_FreeValue(ctx, result);
    JS_FreeValue(ctx, add_func);
    JS_FreeValue(ctx, global_obj);
    JS_FreeContext(ctx);
    JS_FreeRuntime(rt);
    return 1;
  }

  // Print the result
  int32_t sum;
  JS_ToInt32(ctx, &sum, result);
  printf("Result: %d\n", sum);

  JS_FreeValue(ctx, val);
  JS_FreeValue(ctx, result);
  JS_FreeValue(ctx, add_func);
  JS_FreeValue(ctx, global_obj);
  JS_FreeContext(ctx);
  JS_FreeRuntime(rt);

  return 0;
}
