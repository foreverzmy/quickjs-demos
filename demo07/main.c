#include "../helpers/console.c"
#include "../helpers/exception.c"
#include "../helpers/file.c"
#include "../quickjs/quickjs.h"
#include "./fetch.c"
#include <curl/curl.h>
// #include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
  JSRuntime *rt = JS_NewRuntime();
  JSContext *ctx = JS_NewContext(rt);

  // Initialize curl globally
  curl_global_init(CURL_GLOBAL_DEFAULT);

  js_std_init_console(ctx);

  // Initialize the fetch module
  js_init_fetch(ctx);

  // Example JavaScript code that uses fetch
  char *js_code = read_file_to_string("test.js");
  // Evaluate the JavaScript code
  JSValue val = JS_Eval(ctx, js_code, strlen(js_code), "<input>",
                        JS_EVAL_FLAG_STRICT | JS_EVAL_TYPE_MODULE);

  free(js_code);

  if (JS_IsException(val)) {
    check_and_print_exception(ctx);
    return 1;
  }

  JS_FreeValue(ctx, val);

  // Execute the event loop to process any pending promises
  JSContext *ctx1;
  for (;;) {
    int err = JS_ExecutePendingJob(JS_GetRuntime(ctx), &ctx1);
    if (err <= 0) {
      if (err < 0)
        fprintf(stderr, "Error in pending job\n");
      break;
    }
  }

  JS_RunGC(rt);

  JS_FreeContext(ctx);
  JS_FreeRuntime(rt);
  // Cleanup curl
  curl_global_cleanup();

  return 0;
}
