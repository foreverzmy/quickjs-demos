#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <uv.h>

#include "../helpers/console.c"
#include "../helpers/exception.c"
#include "../quickjs/quickjs.h"
#include "./cache.c"
#include "./eventloop.c"

void eval_script(JSContext *ctx, const char *script) {
  JSValue result =
      JS_Eval(ctx, script, strlen(script), "<input>", JS_EVAL_TYPE_MODULE);
  if (JS_IsException(result)) {
    check_and_print_exception(ctx);
  }
  JS_FreeValue(ctx, result);

  // 处理可能产生的异步任务
  process_jobs(ctx);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <js_file1> [<js_file2> ...]\n", argv[0]);
    return 1;
  }

  // 初始化 libuv 事件循环
  init_loop();

  // JS文件数量
  int num_files = argc - 1;

  clock_t start, end;
  start = clock();

  JSRuntime *rt = JS_NewRuntime();

  JSContext **ctxs = malloc(num_files * sizeof(JSContext *));

  char *codes = (char *)calloc(num_files, sizeof(char *));
  for (int i = 0; i < num_files; i++) {
    size_t length = 0;
    const char *filename = argv[i + 1];
    char *js_code = get_file_content(filename, &length);

    ctxs[i] = JS_NewContext(rt);
    codes[i] = *js_code;

    js_std_init_console(ctxs[i]);
    js_std_init_timeout(ctxs[i]);

    eval_script(ctxs[i], js_code);
  }

  uv_run(loop, UV_RUN_DEFAULT);

  // cleanup:
  // 清理并释放资源
  for (int i = 0; i < num_files; i++) {
    JS_FreeContext(ctxs[i]);
  }

  free(codes);
  cleanup_file_cache();
  free(ctxs);
  uv_loop_close(loop);
  JS_FreeRuntime(rt);

  end = clock();
  printf("Total execution time: %.6f seconds.\n",
         ((double)(end - start)) / CLOCKS_PER_SEC);
  return 0;
}
