#include "../demo03/console.c"
#include "../quickjs/quickjs.h"
#include "./car.c"
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
  JSRuntime *rt = JS_NewRuntime();
  JSContext *ctx = JS_NewContext(rt);

  init_std_console(ctx);

  // Initialize Car class
  js_car_init(ctx);

  // Your JavaScript code can now use the Car class
  const char *js_code = "const myCar = new Car();\n"
                        "console.log(myCar.echo('hello world!'));\n";

  JSValue val =
      JS_Eval(ctx, js_code, strlen(js_code), "<input>", JS_EVAL_TYPE_GLOBAL);

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