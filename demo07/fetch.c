#include "../quickjs/quickjs.h"
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Structure to hold response data
typedef struct {
  char *data;
  size_t size;
} ResponseData;

// Callback function for curl to write received data
static size_t write_callback(void *contents, size_t size, size_t nmemb,
                             void *userp) {
  size_t realsize = size * nmemb;
  ResponseData *resp = (ResponseData *)userp;

  // Check for overflow
  if (realsize > SIZE_MAX - resp->size - 1) {
    fprintf(stderr, "Memory size overflow\n");
    return 0;
  }

  char *ptr = realloc(resp->data, resp->size + realsize + 1);
  if (!ptr) {
    fprintf(stderr, "Memory reallocation failed\n");
    return 0;
  }

  resp->data = ptr;
  memcpy(&(resp->data[resp->size]), contents, realsize);
  resp->size += realsize;
  resp->data[resp->size] = '\0';

  return realsize;
}

// The fetch function that will be exposed to JavaScript
static JSValue js_fetch(JSContext *ctx, JSValueConst this_val, int argc,
                        JSValueConst *argv) {
  // Create a Promise
  JSValue promise, resolving_funcs[2];

  promise = JS_NewPromiseCapability(ctx, resolving_funcs);
  if (JS_IsException(promise))
    return JS_EXCEPTION;

  JSValue resolve = resolving_funcs[0];
  JSValue reject = resolving_funcs[1];

  if (argc < 1) {
    JSValue error = JS_NewString(ctx, "URL parameter required");
    JS_Call(ctx, reject, JS_UNDEFINED, 1, &error);
    JS_FreeValue(ctx, error);
    JS_FreeValue(ctx, resolve);
    JS_FreeValue(ctx, reject);
    return promise;
  }

  const char *fetchUrl = JS_ToCString(ctx, argv[0]);
  if (!fetchUrl) {
    JSValue error = JS_NewString(ctx, "Invalid URL");
    JS_Call(ctx, reject, JS_UNDEFINED, 1, &error);
    JS_FreeValue(ctx, error);
    JS_FreeValue(ctx, resolve);
    JS_FreeValue(ctx, reject);
    return promise;
  }

  CURL *curl;
  CURLcode res;
  // Initialize response data with enough space
  ResponseData response_data = {.data = malloc(1), .size = 0};

  if (!response_data.data) {
    fprintf(stderr, "Memory allocation failed\n");
    JSValue error = JS_NewString(ctx, "Failed to allocate memory");
    JS_Call(ctx, reject, JS_UNDEFINED, 1, &error);
    JS_FreeValue(ctx, error);

    JS_FreeValue(ctx, resolve);
    JS_FreeValue(ctx, reject);
    JS_FreeCString(ctx, fetchUrl);

    return promise;
  }

  response_data.data[0] = '\0'; // Ensure null termination

  curl = curl_easy_init();
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, fetchUrl);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response_data);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "QuickJS-Fetch/1.0");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L); // 30 秒超时

    // Perform the request
    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
      // Reject the promise with an error
      JSValue error = JS_NewString(ctx, curl_easy_strerror(res));
      if (!JS_IsException(error)) {
        JS_Call(ctx, reject, JS_UNDEFINED, 1, &error);
        JS_FreeValue(ctx, error);
      }
      free(response_data.data); // 清理资源
      response_data.data = NULL;
    } else {
      // Resolve the promise with the response data
      JSValue response = JS_NewString(ctx, response_data.data);
      JS_Call(ctx, resolve, JS_UNDEFINED, 1, &response);
      JS_FreeValue(ctx, response);
    }
  } else {
    // Reject the promise if curl initialization failed
    JSValue error = JS_NewString(ctx, "Failed to initialize curl");
    JS_Call(ctx, reject, JS_UNDEFINED, 1, &error);
    JS_FreeValue(ctx, error);
  }

  // Clean up
  curl_easy_cleanup(curl);

  // Free memory only if it was allocated
  if (response_data.data) {
    free(response_data.data);
    response_data.data = NULL;
  }

  JS_FreeCString(ctx, fetchUrl);

  // Free the resolve and reject functions
  JS_FreeValue(ctx, resolve);
  JS_FreeValue(ctx, reject);

  return promise;
}

// Register the fetch function in the global object
static int js_init_fetch(JSContext *ctx) {
  JSValue global_obj = JS_GetGlobalObject(ctx);

  // Define the C function in JavaScript environment
  JSValue fetch_func = JS_NewCFunction(ctx, js_fetch, "fetch", 1);

  JS_SetPropertyStr(ctx, global_obj, "fetch", fetch_func);

  JS_FreeValue(ctx, global_obj);
  return 0;
}
