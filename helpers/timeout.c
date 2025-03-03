#include "../quickjs/quickjs-libc.c"
#include "../quickjs/quickjs.h"

// Structure to hold timer information
typedef struct {
  JSValueConst func;
  int64_t timeout;
} JSTimer;

void js_std_init_async(JSRuntime *rt) {
  JSThreadState *ts;
  os_poll_func = js_os_poll;

  ts = malloc(sizeof(*ts));
  if (!ts) {
    fprintf(stderr, "Could not allocate memory for the worker");
    exit(1);
  }

  memset(ts, 0, sizeof(*ts));

  init_list_head(&ts->os_timers);
  ts->next_timer_id = 1;

  JS_SetRuntimeOpaque(rt, ts);
}

void js_std_free_async(JSRuntime *rt) {
  JSThreadState *ts = JS_GetRuntimeOpaque(rt);
  struct list_head *el, *el1;

  list_for_each_safe(el, el1, &ts->os_timers) {
    JSOSTimer *th = list_entry(el, JSOSTimer, link);
    free_timer(rt, th);
  }

  free(ts);
  JS_SetRuntimeOpaque(rt, NULL); /* fail safe */
}

void js_std_init_set_timeout(JSContext *ctx) {
  JSValue global_obj = JS_GetGlobalObject(ctx);

  JS_SetPropertyStr(ctx, global_obj, "setTimeout",
                    JS_NewCFunction(ctx, js_os_setTimeout, "setTimeout", 2));

  JS_SetPropertyStr(
      ctx, global_obj, "clearTimeout",
      JS_NewCFunction(ctx, js_os_clearTimeout, "clearTimeout", 1));

  JS_FreeValue(ctx, global_obj);
}
