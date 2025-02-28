#include "../demo03/console.c"
#include "../quickjs/quickjs.h"
#include "./car.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
  JSRuntime *rt = JS_NewRuntime();
  JSContext *ctx = JS_NewContext(rt);

  init_std_console(ctx);

  // Initialize Car class
  js_car_init(ctx);

  FILE *fp = fopen("point.js", "r");
  if (!fp) {
    fprintf(stderr, "无法打开 point.js 文件\n");
    return 1;
  }

  // 获取文件大小
  fseek(fp, 0, SEEK_END);
  long file_size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  // 分配内存并读取文件内容
  char *js_code = malloc(file_size + 1);
  fread(js_code, 1, file_size, fp);
  js_code[file_size] = '\0';
  fclose(fp);

  JSValue val =
      JS_Eval(ctx, js_code, strlen(js_code), "a.js", JS_EVAL_TYPE_MODULE);

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