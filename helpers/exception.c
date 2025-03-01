#include "../quickjs/quickjs.h"
#include <stdio.h>

void check_and_print_exception(JSContext *ctx) {
  if (JS_IsException(JS_GetException(ctx))) {
    JSValue exc = JS_GetException(ctx);
    const char *str = JS_ToCString(ctx, exc);
    if (str) {
      fprintf(stderr, "Error: %s\n", str);
      JS_FreeCString(ctx, str);
    }
    JS_FreeValue(ctx, exc);
  }
}
