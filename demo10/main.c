#include <string.h>

#include "../helpers/exception.c"
#include "../quickjs/quickjs.h"
#include "../helpers/console.c"

/**
 * Compiles JavaScript code to bytecode
 *
 * @param js_code The JavaScript code to compile
 * @param out_buf_len Pointer to store the length of the output buffer
 * @return Pointer to the compiled bytecode, or NULL on error
 */
uint8_t *compile_js_to_bytecode(const char *js_code, size_t *out_buf_len)
{
  JSRuntime *rt = JS_NewRuntime();
  JSContext *ctx = JS_NewContext(rt);
  // js_std_init_console(ctx);
  JSValue obj = JS_Eval(ctx, js_code, strlen(js_code), "<input>", JS_EVAL_FLAG_COMPILE_ONLY | JS_EVAL_TYPE_GLOBAL);
  if (JS_IsException(obj))
  {
    check_and_print_exception(ctx);
    return NULL;
  }
  uint8_t *out_buf;
  out_buf = JS_WriteObject(ctx, out_buf_len, obj, JS_WRITE_OBJ_BYTECODE);
  if (!out_buf)
  {
    check_and_print_exception(ctx);
    return NULL;
  }
  JS_FreeValue(ctx, obj);
  JS_FreeContext(ctx);
  JS_FreeRuntime(rt);
  return out_buf;
}

/**
 * Executes compiled bytecode and tests the loaded JavaScript functions
 *
 * @param bytecode Pointer to the compiled bytecode
 * @param bytecode_len Length of the bytecode
 * @return 0 on success, 1 on error
 */
int execute_bytecode(JSRuntime *rt, uint8_t *bytecode, size_t bytecode_len)
{
  JSContext *ctx = JS_NewContext(rt);
  js_std_init_console(ctx);

  // Load bytecode
  JSValue loadedVal = JS_ReadObject(ctx, bytecode, bytecode_len, JS_READ_OBJ_BYTECODE);
  js_free(ctx, bytecode);
  if (JS_IsException(loadedVal))
  {
    check_and_print_exception(ctx);
    return 1;
  }

  // Execute loaded bytecode
  JSValue ret = JS_EvalFunction(ctx, loadedVal);
  if (JS_IsException(ret))
  {
    check_and_print_exception(ctx);
    return 1;
  }

  // Test the loaded add function
  JSValue global_obj = JS_GetGlobalObject(ctx);
  JSValue add_func = JS_GetPropertyStr(ctx, global_obj, "add");
  JSValue args[2];
  args[0] = JS_NewInt32(ctx, 3);
  args[1] = JS_NewInt32(ctx, 4);
  JSValue result = JS_Call(ctx, add_func, global_obj, 2, args);

  if (!JS_IsException(result))
  {
    int32_t num;
    JS_ToInt32(ctx, &num, result);
    printf("add(3, 4) = %d\n", num);
  }
  else
  {
    check_and_print_exception(ctx);
  }

  // Clean up resources
  JS_FreeValue(ctx, args[0]);
  JS_FreeValue(ctx, args[1]);
  JS_FreeValue(ctx, result);
  JS_FreeValue(ctx, add_func);
  JS_FreeValue(ctx, global_obj);
  JS_FreeValue(ctx, ret);
  JS_FreeContext(ctx);

  return 0;
}

int main()
{
  // Execute JavaScript code that uses the C function
  const char *js_code = "function add(a, b) { return a + b; }\n"
                        "function multiply(a, b) { return a * b; }\n"
                        "var message = 'Hello from compiled code!';\n"
                        "console.log(message);\n";

  uint8_t *bytecode;
  size_t bytecode_len;

  bytecode = compile_js_to_bytecode(js_code, &bytecode_len);

  if (!bytecode)
  {
    return 1;
  }

  JSRuntime *rt = JS_NewRuntime();
  // Execute the bytecode
  int ret = execute_bytecode(rt, bytecode, bytecode_len);

  JS_FreeRuntime(rt);
  return ret;
}
