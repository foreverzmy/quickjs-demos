#include <string.h>
#include <time.h>
#include <mach/mach.h>  

#include "../helpers/file.c"
#include "../helpers/exception.c"
#include "../quickjs/quickjs.h"
#include "../helpers/console.c"

// 获取当前进程的内存使用量（以字节为单位）
size_t get_memory_usage() {
  struct task_basic_info info;
  mach_msg_type_number_t size = TASK_BASIC_INFO_COUNT;
  kern_return_t kerr = task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)&info, &size);
  if (kerr == KERN_SUCCESS) {
    return info.resident_size;  // 返回物理内存使用量
  } else {
    return 0;
  }
}

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

int validBenchmarkFunc(JSContext *ctx)
{
  // Test the benchmark results
  JSValue global_obj = JS_GetGlobalObject(ctx);
  JSValue benchmark_result = JS_GetPropertyStr(ctx, global_obj, "benchmarkResult");

  // Check if benchmarkResult exists
  if (JS_IsUndefined(benchmark_result) || JS_IsNull(benchmark_result))
  {
    fprintf(stderr, "Error: benchmarkResult not found\n");
    JS_FreeValue(ctx, benchmark_result);
    JS_FreeValue(ctx, global_obj);
    return 1;
  }

  // Check if combinedResult property exists and is a number
  JSValue combined_result = JS_GetPropertyStr(ctx, benchmark_result, "combinedResult");
  if (!JS_IsNumber(combined_result))
  {
    fprintf(stderr, "Error: benchmarkResult.combinedResult is not a number\n");
    JS_FreeValue(ctx, combined_result);
    JS_FreeValue(ctx, benchmark_result);
    JS_FreeValue(ctx, global_obj);
    return 1;
  }

  // Get combinedResult value
  double result_value;
  JS_ToFloat64(ctx, &result_value, combined_result);

  // Get totalTimeMs value for logging
  JSValue total_time = JS_GetPropertyStr(ctx, benchmark_result, "totalTimeMs");
  double time_value = 0;
  JS_ToFloat64(ctx, &time_value, total_time);

  // printf("Benchmark completed with result: %.2f in %.2f ms\n", result_value, time_value);

  // Clean up resources
  JS_FreeValue(ctx, total_time);
  JS_FreeValue(ctx, combined_result);
  JS_FreeValue(ctx, benchmark_result);
  JS_FreeValue(ctx, global_obj);

  // Test is valid if we got a positive number as result
  return result_value > 0 ? 0 : 1;
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
  // js_std_init_console(ctx);

  // Load bytecode
  JSValue loadedVal = JS_ReadObject(ctx, bytecode, bytecode_len, JS_READ_OBJ_BYTECODE);
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

  int v = validBenchmarkFunc(ctx);

  JS_FreeValue(ctx, ret);
  JS_FreeContext(ctx);

  return v;
}

int execute_js(JSRuntime *rt, const char *js_code)
{
  JSContext *ctx = JS_NewContext(rt);
  // js_std_init_console(ctx);

  JSValue val =
      JS_Eval(ctx, js_code, strlen(js_code), "<input>", JS_EVAL_TYPE_GLOBAL);

  if (JS_IsException(val))
  {
    check_and_print_exception(ctx);
    return 1;
  }

  int v = validBenchmarkFunc(ctx);

  JS_FreeValue(ctx, val);
  JS_FreeContext(ctx);

  return v;
}

// Execute JavaScript code that uses the C function
int benchmark_js(int iterations ) {
  printf("\nTesting execute_js performance...\n");
  const char *js_code = read_file_to_string("./benchmark.js");
  JSRuntime *rt = JS_NewRuntime();

   // Variables for timing
  clock_t start, end;
  double cpu_time_used_s, cpu_time_used_ms;

  // Variables for memory usage
  size_t mem_before, mem_after, mem_used;

  mem_before = get_memory_usage();

  start = clock();
  // mem_before = get_memory_usage();

  for (int i = 0; i < iterations; i++)
  {
    int ret = execute_js(rt, js_code);
    if (ret == 1)
    {
      printf("Failed to execute JS\n");
      JS_FreeRuntime(rt);
      return 1;
    }
  }

  end = clock();
  cpu_time_used_s = ((double)(end - start)) / CLOCKS_PER_SEC;
  cpu_time_used_ms = ((double)(end - start)) / 1000;
  mem_after = get_memory_usage();
  mem_used = mem_after > mem_before ? mem_after - mem_before : 0;
  printf("execute_js: %f seconds (total for %d iterations)\n", cpu_time_used_s, iterations);
  printf("execute_js: %f milliseconds (average per iteration)\n", cpu_time_used_ms / iterations);
  printf("execute_js: %zu bytes memory used\n", mem_used);
  printf("execute_js: %.2f KB memory used\n", mem_used / 1024.0);

  JS_FreeRuntime(rt);

  return 0;
}

// Test execute_bytecode performance
int benchmark_bytecode(int iterations) {
  printf("\nTesting execute_bytecode performance...\n");
  const char *js_code = read_file_to_string("./benchmark.js");
  JSRuntime *rt = JS_NewRuntime();

  uint8_t *bytecode;
  size_t bytecode_len;
  bytecode = compile_js_to_bytecode(js_code, &bytecode_len);
  if (!bytecode)
  {
    printf("Failed to compile bytecode\n");
    return 1;
  }

  // Variables for timing
  clock_t start, end;
  double cpu_time_used_s, cpu_time_used_ms;

  // Variables for memory usage
  size_t mem_before, mem_after, mem_used;

  mem_before = get_memory_usage();

  rt = JS_NewRuntime();

  start = clock();
  // mem_before = get_memory_usage();

  for (int i = 0; i < iterations; i++)
  {
    int ret = execute_bytecode(rt, bytecode, bytecode_len);
    if (ret == 1)
    {
      printf("Failed to execute bytecode\n");
      JS_FreeRuntime(rt);
      return 1;
    }
  }

  end = clock();
  cpu_time_used_s = ((double)(end - start)) / CLOCKS_PER_SEC;
  cpu_time_used_ms = ((double)(end - start)) / 1000;
  mem_after = get_memory_usage();
  mem_used = mem_after > mem_before ? mem_after - mem_before : 0;

  printf("execute_bytecode: %f seconds (total for %d iterations)\n", cpu_time_used_s, iterations);
  printf("execute_bytecode: %f milliseconds (average per iteration)\n", cpu_time_used_ms / iterations);
  printf("execute_bytecode: %zu bytes memory used\n", mem_used);
  printf("execute_bytecode: %.2f KB memory used\n", mem_used / 1024.0);

  // Clean up
  free(bytecode);
  JS_FreeRuntime(rt);

  return 0;
}

int main()
{
  // Number of iterations for more reliable measurements
  const int iterations = 1000;
  int ret = 0;

  ret = benchmark_js(iterations);
  if (ret == 1) {
    return 1;
  }

  ret = benchmark_bytecode(iterations);
  if (ret == 1) {
    return 1;
  }

  return 0;
}
