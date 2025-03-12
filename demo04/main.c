#include <string.h>

#include "../helpers/console.c"
#include "../helpers/exception.c"
#include "../quickjs/quickjs.h"
#include "./vendor.c"

int main(int argc, char **argv)
{
  JSRuntime *rt = JS_NewRuntime();
  JSContext *ctx = JS_NewContext(rt);

  js_std_init_console(ctx);

  // Initialize Vendor class
  js_init_vendor_class(ctx);

  // Your JavaScript code can now use the Car class
  const char *js_code = "const v = new Vendor('Mervyn');\n"
                        "console.log('==log 1===', v.echo());\n"
                        "console.log('==log 2===', v.echo());\n";

  JSValue val =
      JS_Eval(ctx, js_code, strlen(js_code), "<input>", JS_EVAL_TYPE_GLOBAL);

  if (JS_IsException(val))
  {
    check_and_print_exception(ctx);
    return 1;
  }

  JS_FreeValue(ctx, val);

  JS_RunGC(rt);

  JS_FreeContext(ctx);
  JS_FreeRuntime(rt);
  return 0;
}