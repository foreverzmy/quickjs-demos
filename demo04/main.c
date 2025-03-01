#include "../demo03/console.c"
#include "../quickjs/quickjs.h"
#include "./vendor.c"
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
  JSRuntime *rt = JS_NewRuntime();
  JSContext *ctx = JS_NewContext(rt);

  init_std_console(ctx);

  // Initialize Vendor class
  js_init_vendor_class(ctx);

  // Your JavaScript code can now use the Car class
  const char *js_code = "const v = new Vendor('Mervyn');\n"
                        "console.log('==log 1===', v.echo());\n"
                        "console.log('==log 2===', v.echo());\n";

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